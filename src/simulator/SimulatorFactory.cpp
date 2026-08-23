// ------------------------------------------------
// SimulatorFactory.hppの実装
// ------------------------------------------------

#include "SimulatorFactory.hpp"

#include <filesystem>
#include <stdexcept>
#include <utility>

#include "app/CommandLine.hpp"
#include "app/InputDiagnosticPrinter.hpp"
#include "app/Option.hpp"
#include "io/SimulationInputReader.hpp"
#include "simulator/DetailSimulator.hpp"
#include "simulator/ScatterSimulator.hpp"

namespace SimulatorFactory {
    const std::string specDirectoryPath = "input/spec/";

    namespace {
        std::string setSpecFile(const CommandLineOption::Option& option) {
            if (option.specifySpecFile) {
                if (!std::filesystem::exists(specDirectoryPath + option.specFilePath)) {
                    // Terminate if the specified file does not exist
                    throw std::runtime_error{"Specified spec file does not exist: " + option.specFilePath};
                }
                CommandLine::PrintInfo(PrintInfoType::Information,
                                         "Using specified spec file: " + option.specFilePath);
                return specDirectoryPath + option.specFilePath;
            }

            std::vector<std::string> specificationFiles;

            for (const std::filesystem::directory_entry& x : std::filesystem::directory_iterator(specDirectoryPath)) {
                specificationFiles.push_back(x.path().filename().string());
            }

            if (specificationFiles.size() == 0) {
                throw std::runtime_error{"Specification file not found in input/spec/."};
            }

			// Sort files by ascending order
			std::sort(specificationFiles.begin(), specificationFiles.end());

            std::cout << "<!===Set Specification File===!>" << std::endl;

            for (size_t i = 0; i < specificationFiles.size(); i++) {
                std::cout << i + 1 << ": " << specificationFiles[i] << std::endl;
            }

            const size_t inputIndex = CommandLine::InputIndex<size_t>(specificationFiles.size());
            return specDirectoryPath + specificationFiles[inputIndex - 1];
        }

        SimulationMode setSimulationMode() {
            CommandLine::Question("Set Simulation Mode", "Scatter Mode", "Detail Mode");
            return CommandLine::InputIndex<SimulationMode>(2);
        }

        TrajectoryMode setTrajectoryMode() {
            CommandLine::Question("Set Falling Type", "Trajectory", "Parachute");
            return CommandLine::InputIndex<TrajectoryMode>(2);
        }

        std::pair<double, double> setWindCondition() {
            double windSpeed = 0, windDirection = 0;

            std::cout << "Input Wind Velocity[m/s]" << std::endl;
            std::cin >> windSpeed;
            std::cout << std::endl;

            std::cout << "Input Wind Direction[deg] (North: 0, East: 90)" << std::endl;
            std::cin >> windDirection;
            std::cout << std::endl;

            return {windSpeed, windDirection};
        }

        DetachType setDetachType() {
            CommandLine::Question("Set Detach Type",
                                  "When burning finished",
                                  "Specify time",
                                  "Concurrently with parachute",
                                  "Do not detach");
            return CommandLine::InputIndex<DetachType>(4);
        }

        double setDetachTime() {
            double time = 0;

            CommandLine::Question("Set Detach Time");
            std::cin >> time;
            std::cout << std::endl;

            return time;
        }

        SimulationMode SetupSimulationInput(SimulationInput& input,
                                            const ApplicationSettings& applicationSettings) {
            SimulationMode simulationMode = SimulationMode::Detail;

            if (applicationSettings.solver.wind.type != WindModelType::Real
                && applicationSettings.solver.wind.type != WindModelType::NoWind) {
                simulationMode = setSimulationMode();
            }

            input.run.trajectoryMode = setTrajectoryMode();
            input.run.windDirection.reference = DirectionReference::MagneticNorth;

            // Set wind condition if need
            if (simulationMode == SimulationMode::Detail
                && applicationSettings.solver.wind.type != WindModelType::Real
                && applicationSettings.solver.wind.type != WindModelType::NoWind) {
                std::tie(input.run.windSpeed, input.run.windDirection.degrees) = setWindCondition();
            }

            // Setup multiple rocket
            if (input.rocket.isMultiple()) {
                CommandLine::PrintInfo(PrintInfoType::Information, "This is Multiple Rocket");
                input.run.detachType = setDetachType();
                if (input.run.detachType == DetachType::Time) {
                    input.run.detachTime = setDetachTime();
                }
            }

            return simulationMode;
        }
    }

    std::unique_ptr<SimulatorBase> Create(const CommandLineOption::Option& option,
                                          const ApplicationSettings& applicationSettings) {
        try {
            // Specification json file
            const auto specFilePath = setSpecFile(option);
            const SimulationInputReader::Document inputDocument(specFilePath);

            auto inputReadResult = inputDocument.toSimulationInput();
            InputDiagnosticPrinter::Print(inputReadResult.diagnostics);
            auto input = std::move(inputReadResult.value);
            const SimulationMode simulationMode = SetupSimulationInput(input, applicationSettings);

            // Create simulator instance
            switch (simulationMode) {
            case SimulationMode::Detail:
                return std::make_unique<DetailSimulator>(
                    inputDocument.specificationName(), std::move(input), simulationMode, applicationSettings);
            case SimulationMode::Scatter:
                return std::make_unique<ScatterSimulator>(
                    inputDocument.specificationName(), std::move(input), simulationMode, applicationSettings);
            }

            throw std::runtime_error{"SimulatorFactory::Create(): Detected unhandled return path."};
        } catch (const std::exception& e) {
            CommandLine::PrintInfo(PrintInfoType::Error, e.what());
            return nullptr;
        }
    }
}
