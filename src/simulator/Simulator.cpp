#include "Simulator.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "DetailSimulator.hpp"
#include "ScatterSimulator.hpp"
#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "env/Map.hpp"
#include "rocket/RocketSpec.hpp"

Simulator* Simulator::New(double dt) {
    Simulator* simulator = nullptr;

    const auto jsonFile = SetJSONFile();

    if (AppSetting::WindModel::type == WindModelType::Real) {
        simulator = new DetailSimulator(jsonFile, dt);
    } else {
        switch (SetSimulationMode()) {
        case SimulationMode::Detail:
            simulator = new DetailSimulator(jsonFile, dt);
            break;
        case SimulationMode::Scatter:
            simulator = new ScatterSimulator(jsonFile, dt);
            break;
        }
    }

    return simulator;
}

bool Simulator::initialize() {
    // settings on commandline
    setTrajectoryMode();

    if (AppSetting::WindModel::type != WindModelType::Real && m_simulationMode == SimulationMode::Detail) {
        setWindCondition();
    }

    if (RocketSpec::IsMultipleRocket(m_jsonFile)) {
        CommandLine::PrintInfo(PrintInfoType::Information, "This is Multiple Rocket");
        m_rocketType = RocketType::Multi;
        setDetachType();
        if (m_detachType == DetachType::Time) {
            setDetachTime();
        }
    } else {
        m_rocketType = RocketType::Single;
    }

    std::cout << "----------------------------------------------------------" << std::endl;

    // read json
    m_environment.initialize(m_jsonFile);
    m_rocketSpec.initialize(m_jsonFile);

    // output
    m_outputDirName = m_jsonFile;
    m_outputDirName.erase(m_outputDirName.size() - 5, 5);
    m_outputDirName += "[";

    const std::filesystem::path f = AppSetting::WindModel::realdataFilename;
    switch (AppSetting::WindModel::type) {
    case WindModelType::Real:
        m_outputDirName += "(" + f.stem().string() + ")";
        break;
    case WindModelType::Original:
        m_outputDirName += "original";
        break;
    case WindModelType::OnlyPowerLow:
        m_outputDirName += "powerlow";
        break;
    }

    switch (m_simulationMode) {
    case SimulationMode::Scatter:
        m_outputDirName += "_scatter";
        break;
    case SimulationMode::Detail:
        m_outputDirName += "_detail";
        break;
    }

    switch (m_trajectoryMode) {
    case TrajectoryMode::Parachute:
        m_outputDirName += "_para";
        break;
    case TrajectoryMode::Trajectory:
        m_outputDirName += "_trajectory";
        break;
    }

    m_outputDirName += "]";

    if (m_simulationMode == SimulationMode::Detail) {
        std::ostringstream out;
        out.precision(2);
        out << std::fixed << m_windSpeed << "ms, " << m_windDirection << "deg";
        m_outputDirName += "[" + out.str() + "]";
    }

    return true;
}

bool Simulator::run() {
    if (!initialize()) {
        CommandLine::PrintInfo(PrintInfoType::Error, "Could not initialize Simulator");
        return false;
    }

    createResultDirectory();

    // Set map
    {
        auto place = m_environment.place;
        std::transform(
            place.begin(), place.end(), place.begin(), [](int c) { return static_cast<char>(::tolower(c)); });
        if (const auto map = Map::GetMap(place); map.has_value()) {
            m_mapData = map.value();
            Gnuplot::Initialize(m_outputDirName.c_str(), m_mapData);
        } else {
            CommandLine::PrintInfo(PrintInfoType::Error, "This map is invalid.");
            return false;
        }
    }

    // Simulate
    {
        const auto start = std::chrono::system_clock::now();

        if (!simulate()) {
            return false;
        }

        const auto end      = std::chrono::system_clock::now();
        const auto elapsed  = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        const std::string s = "Finish processing: " + std::to_string(elapsed / 1000000.0) + "[s]";
        CommandLine::PrintInfo(PrintInfoType::Information, s.c_str());
    }

    // Save result and init commandline
    {
        saveResult();

        const std::string str = "Result is saved in \"" + m_outputDirName + "/\"";
        CommandLine::PrintInfo(PrintInfoType::Information, str.c_str());

        CommandLine::SetOutputDir(m_outputDirName);
    }

    return true;
}

std::string Simulator::SetJSONFile() {
    std::cout << "<!===Set JSON File===!>" << std::endl;
    std::vector<std::string> existJSONs;

    for (const std::filesystem::directory_entry& x : std::filesystem::directory_iterator("input/json")) {
        existJSONs.push_back(x.path().filename().string());
    }

    for (size_t i = 0; i < existJSONs.size(); i++) {
        std::cout << i + 1 << ": " << existJSONs[i] << std::endl;
    }

    const size_t jsonIndex = CommandLine::InputIndex<size_t>(existJSONs.size());
    return existJSONs[jsonIndex - 1];
}

SimulationMode Simulator::SetSimulationMode() {
    CommandLine::Question("Set Simulation Mode", "Scatter Mode", "Detail Mode");
    return CommandLine::InputIndex<SimulationMode>(2);
}

void Simulator::setTrajectoryMode() {
    CommandLine::Question("Set Falling Type", "Trajectory", "Parachute");
    m_trajectoryMode = CommandLine::InputIndex<TrajectoryMode>(2);
}

void Simulator::setWindCondition() {
    std::cout << "Input Wind Velocity[m/s]" << std::endl;
    std::cin >> m_windSpeed;
    std::cout << std::endl;

    std::cout << "Input Wind Direction[deg] (North: 0, East: 90)" << std::endl;
    std::cin >> m_windDirection;
    std::cout << std::endl;
}

void Simulator::setDetachType() {
    CommandLine::Question(
        "Set Detach Type", "When burning finished", "Specify time", "Concurrently with parachute", "Do not detach");
    m_detachType = CommandLine::InputIndex<DetachType>(4);
}

void Simulator::setDetachTime() {
    CommandLine::Question("Set Detach Time");
    std::cin >> m_detachTime;
    std::cout << std::endl;
}

void Simulator::createResultDirectory() {
    const std::filesystem::path result = "result";
    if (!std::filesystem::exists(result)) {
        if (!std::filesystem::create_directory(result)) {
            CommandLine::PrintInfo(PrintInfoType::Error, "Failed to create result directory");
        }
    }

    const std::filesystem::path output = "result/" + m_outputDirName;
    if (!std::filesystem::exists(output)) {
        if (!std::filesystem::create_directory(output)) {
            CommandLine::PrintInfo(PrintInfoType::Error, "Failed to create result directory");
        }
    }
}
