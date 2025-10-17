// ------------------------------------------------
// SimulatorFactory.hppの実装
// ------------------------------------------------

#include "SimulatorFactory.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <filesystem>
#include <stdexcept>

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "app/Option.hpp"
#include "simulator/DetailSimulator.hpp"
#include "simulator/ScatterSimulator.hpp"

namespace SimulatorFactory {
    const std::string specDirectoryPath = "input/spec/";

    namespace _internal {
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

        SimulatorBase::SimulationSetting SetupSimulator(const boost::property_tree::ptree& specJson) {
            SimulatorBase::SimulationSetting setting;

            if (AppSetting::WindModel::type != WindModelType::Real
                && AppSetting::WindModel::type != WindModelType::NoWind) {
                setting.simulationMode = _internal::setSimulationMode();
            }

            setting.trajectoryMode = _internal::setTrajectoryMode();

            // Set wind condition if need
            if (setting.simulationMode == SimulationMode::Detail && AppSetting::WindModel::type != WindModelType::Real
                && AppSetting::WindModel::type != WindModelType::NoWind) {
                std::tie(setting.windSpeed, setting.windDirection) = setWindCondition();
            }

            // Setup multiple rocket
            if (RocketSpecification::IsMultipleRocket(specJson)) {
                CommandLine::PrintInfo(PrintInfoType::Information, "This is Multiple Rocket");
                setting.detachType = setDetachType();
                if (setting.detachType == DetachType::Time) {
                    setting.detachTime = setDetachTime();
                }
            }

            return setting;
        }
    }

    std::unique_ptr<SimulatorBase> Create(const CommandLineOption::Option& option) {
        try {
            // Specification json file
            const auto specFilePath = _internal::setSpecFile(option);
            boost::property_tree::ptree specJson;
            boost::property_tree::read_json(specFilePath, specJson);

            // Specification name
            std::string specName = specFilePath;
            specName.erase(0, specDirectoryPath.size());
            specName.erase(specName.size() - 5, 5);

            // Setup simulator
            const auto simulationSetting = _internal::SetupSimulator(specJson);

            // Create simulator instance
            if (AppSetting::WindModel::type == WindModelType::Real
                || AppSetting::WindModel::type == WindModelType::NoWind) {
                return std::make_unique<DetailSimulator>(specName, specJson, simulationSetting);
            } else {
                switch (simulationSetting.simulationMode) {
                case SimulationMode::Detail:
                    return std::make_unique<DetailSimulator>(specName, specJson, simulationSetting);
                case SimulationMode::Scatter:
                    return std::make_unique<ScatterSimulator>(specName, specJson, simulationSetting);
                }
            }

            throw std::runtime_error{"SimulatorFactory::Create(): Detected unhandled return path."};
        } catch (const std::exception& e) {
            CommandLine::PrintInfo(PrintInfoType::Error, e.what());
            return nullptr;
        }
    }
}
