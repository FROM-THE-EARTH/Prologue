// ------------------------------------------------
// SimulationRequest.hppの実装
// ------------------------------------------------

#include "SimulationRequest.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

#include "cli/CommandLine.hpp"
#include "io/FileProjectResourceProvider.hpp"
#include "io/MeasuredWindProfileReader.hpp"
#include "io/ProjectJsonSerializer.hpp"
#include "project/SimulationInputBuilder.hpp"

namespace cli {
    namespace {
        const std::filesystem::path SpecificationDirectory = "input/spec";

        std::filesystem::path SelectSpecificationFile(const CommandLineOption::Option& option) {
            if (option.specifySpecFile) {
                const auto specificationFile = SpecificationDirectory / option.specFilePath;
                if (!std::filesystem::exists(specificationFile)) {
                    throw std::runtime_error{"Specified spec file does not exist: " + option.specFilePath};
                }
                CommandLine::PrintInfo(PrintInfoType::Information,
                                       "Using specified spec file: " + option.specFilePath);
                return specificationFile;
            }

            std::vector<std::filesystem::path> specificationFiles;
            for (const auto& entry : std::filesystem::directory_iterator(SpecificationDirectory)) {
                specificationFiles.push_back(entry.path());
            }
            if (specificationFiles.empty()) {
                throw std::runtime_error{"Specification file not found in input/spec/."};
            }
            std::sort(specificationFiles.begin(), specificationFiles.end());

            std::cout << "<!===Set Specification File===!>" << std::endl;
            for (size_t i = 0; i < specificationFiles.size(); i++) {
                std::cout << i + 1 << ": " << specificationFiles[i].filename().string() << std::endl;
            }

            const size_t inputIndex = CommandLine::InputIndex<size_t>(specificationFiles.size());
            return specificationFiles[inputIndex - 1];
        }

        SimulationMode SelectSimulationMode() {
            CommandLine::Question("Set Simulation Mode", "Scatter Mode", "Detail Mode");
            return CommandLine::InputIndex<SimulationMode>(2);
        }

        TrajectoryMode SelectTrajectoryMode() {
            CommandLine::Question("Set Falling Type", "Trajectory", "Parachute");
            return CommandLine::InputIndex<TrajectoryMode>(2);
        }

        std::pair<double, double> ReadWindCondition() {
            double windSpeed = 0.0;
            double windDirection = 0.0;
            std::cout << "Input Wind Velocity[m/s]" << std::endl;
            std::cin >> windSpeed;
            std::cout << std::endl;
            std::cout << "Input Wind Direction[deg] (North: 0, East: 90)" << std::endl;
            std::cin >> windDirection;
            std::cout << std::endl;
            return {windSpeed, windDirection};
        }

        DetachType SelectDetachType() {
            CommandLine::Question("Set Detach Type",
                                  "When burning finished",
                                  "Specify time",
                                  "Concurrently with parachute",
                                  "Do not detach");
            return CommandLine::InputIndex<DetachType>(4);
        }

        double ReadDetachTime() {
            double time = 0.0;
            CommandLine::Question("Set Detach Time");
            std::cin >> time;
            std::cout << std::endl;
            return time;
        }

        SimulationMode ConfigureRun(SimulationInput& input,
                                    const ApplicationSettings& applicationSettings) {
            SimulationMode mode = SimulationMode::Detail;
            if (applicationSettings.solver.wind.type != WindModelType::Real
                && applicationSettings.solver.wind.type != WindModelType::NoWind) {
                mode = SelectSimulationMode();
            }

            input.run.trajectoryMode = SelectTrajectoryMode();
            input.run.windDirection.reference = DirectionReference::MagneticNorth;

            if (mode == SimulationMode::Detail
                && applicationSettings.solver.wind.type != WindModelType::Real
                && applicationSettings.solver.wind.type != WindModelType::NoWind) {
                std::tie(input.run.windSpeed, input.run.windDirection.degrees) = ReadWindCondition();
            }

            if (input.rocket.isMultiple()) {
                CommandLine::PrintInfo(PrintInfoType::Information, "This is Multiple Rocket");
                input.run.detachType = SelectDetachType();
                if (input.run.detachType == DetachType::Time) {
                    input.run.detachTime = ReadDetachTime();
                }
            }
            return mode;
        }
    }

    SimulationRequest PrepareSimulation(const CommandLineOption::Option& option,
                                        const ApplicationSettings& applicationSettings) {
        const auto specificationFile = SelectSpecificationFile(option);
        const auto document = ProjectJsonSerializer::Load(specificationFile);
        const ProjectIO::FileProjectResourceProvider resources(specificationFile.parent_path());
        auto input = project::BuildSimulationInput(document, resources);

        input.solver = applicationSettings.solver;
        if (input.solver.wind.type == WindModelType::Real) {
            input.solver.wind.measuredProfile = MeasuredWindProfileReader::Read(
                std::filesystem::path("input/wind") / applicationSettings.measuredWindFilename);
        }

        const SimulationMode mode = ConfigureRun(input, applicationSettings);
        return {
            .specificationName = specificationFile.stem().string(),
            .mode = mode,
            .input = std::move(input),
        };
    }
}
