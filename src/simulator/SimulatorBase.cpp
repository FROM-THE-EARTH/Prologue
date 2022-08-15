// ------------------------------------------------
// SimulatorBase.hppの実装
// ------------------------------------------------

#include "SimulatorBase.hpp"

#include <chrono>
#include <filesystem>
#include <stdexcept>

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "env/Environment.hpp"
#include "env/Map.hpp"
#include "rocket/RocketSpec.hpp"

SimulatorBase::SimulatorBase(const std::string specName,
                             const boost::property_tree::ptree& specJson,
                             const SimulationSetting& setting) :
    m_specName(specName),
    m_setting(setting),
    m_rocketType(RocketSpecification::IsMultipleRocket(specJson) ? RocketType::Multi : RocketType::Single),
    m_rocketSpec(RocketSpecification(specJson)),
    m_environment(Environment(specJson)),
    m_mapData(getMapData()),
    m_outputDirName(getOutputDirectoryName()) {}

bool SimulatorBase::run(bool output) {
    createResultDirectory();

    // Simulate
    {
        const auto start = std::chrono::system_clock::now();

        if (!simulate()) {
            return false;
        }

        const auto end     = std::chrono::system_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        CommandLine::PrintInfo(PrintInfoType::Information,
                               "Finish processing: " + std::to_string(elapsed / 1000000.0) + "[s]");
    }

    // Save result and init commandline
    if (output) {
        CommandLine::PrintInfo(PrintInfoType::Information, "Saving result...");

        saveResult();

        CommandLine::PrintInfo(PrintInfoType::Information, "Result is saved in \"" + m_outputDirName + "/\"");
    }

    return true;
}

void SimulatorBase::createResultDirectory() {
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

std::string SimulatorBase::getOutputDirectoryName() {
    std::string dir = m_specName;

    dir += "[";

    const std::filesystem::path realWindFile = AppSetting::WindModel::realdataFilename;

    switch (AppSetting::WindModel::type) {
    case WindModelType::Real:
        dir += "(" + realWindFile.stem().string() + ")";
        break;
    case WindModelType::Original:
        dir += "original";
        break;
    case WindModelType::OnlyPowerLow:
        dir += "powerlow";
        break;
    case WindModelType::NoWind:
        dir += "nowind";
        break;
    }

    if (AppSetting::WindModel::type != WindModelType::Real) {
        switch (m_setting.simulationMode) {
        case SimulationMode::Scatter:
            dir += "_scatter";
            break;
        case SimulationMode::Detail:
            dir += "_detail";
            break;
        }
    }

    switch (m_setting.trajectoryMode) {
    case TrajectoryMode::Parachute:
        dir += "_para";
        break;
    case TrajectoryMode::Trajectory:
        dir += "_traj";
        break;
    }

    dir += "]";

    if (m_setting.simulationMode == SimulationMode::Detail && AppSetting::WindModel::type != WindModelType::Real
        && AppSetting::WindModel::type != WindModelType::NoWind) {
        std::ostringstream out;
        out.precision(2);
        out << std::fixed << m_setting.windSpeed << "ms, " << m_setting.windDirection << "deg";
        dir += "[" + out.str() + "]";
    }

    return dir;
}

MapData SimulatorBase::getMapData() {
    // Get / Set place
    std::string place = m_environment.place;
    std::transform(place.begin(), place.end(), place.begin(), [](int c) { return static_cast<char>(::tolower(c)); });
    if (const auto map = Map::GetMap(place); map.has_value()) {
        return map.value();
    } else {
        throw std::runtime_error{"This map is invalid."};
    }
}
