// ------------------------------------------------
// SimulatorBase.hppの実装
// ------------------------------------------------

#include "SimulatorBase.hpp"

#include <chrono>
#include <filesystem>
#include <stdexcept>
#include <utility>

#include "app/CommandLine.hpp"
#include "env/Map.hpp"
#include "io/MeasuredWindProfileReader.hpp"

SimulatorBase::SimulatorBase(std::string specificationName,
                             SimulationInput input,
                             SimulationMode simulationMode,
                             ApplicationSettings applicationSettings) :
    m_specName(std::move(specificationName)),
    m_applicationSettings(std::move(applicationSettings)),
    m_simulationMode(simulationMode),
    m_input(std::move(input)),
    m_mapData(getMapData()) {
    m_input.solver = getSolverSettings();
    m_input.run.magneticDeclination = m_mapData.magneticDeclination;
    m_outputDirName = getOutputDirectoryName();
}

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

std::string SimulatorBase::getOutputDirectoryName() const {
    std::string dir = m_specName;

    dir += "[";

    const std::filesystem::path realWindFile = m_applicationSettings.measuredWindFilename;

    switch (m_input.solver.wind.type) {
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

    if (m_input.solver.wind.type != WindModelType::Real) {
        switch (m_simulationMode) {
        case SimulationMode::Scatter:
            dir += "_scatter";
            break;
        case SimulationMode::Detail:
            dir += "_detail";
            break;
        }
    }

    switch (m_input.run.trajectoryMode) {
    case TrajectoryMode::Parachute:
        dir += "_para";
        break;
    case TrajectoryMode::Trajectory:
        dir += "_traj";
        break;
    }

    dir += "]";

    if (m_simulationMode == SimulationMode::Detail && m_input.solver.wind.type != WindModelType::Real
        && m_input.solver.wind.type != WindModelType::NoWind) {
        std::ostringstream out;
        out.precision(2);
        out << std::fixed << m_input.run.windSpeed << "ms, " << m_input.run.windDirection.degrees << "deg";
        dir += "[" + out.str() + "]";
    }

    return dir;
}

MapData SimulatorBase::getMapData() const {
    // Get / Set place
    std::string place = m_input.environment.place;
    std::transform(place.begin(), place.end(), place.begin(), [](int c) { return static_cast<char>(::tolower(c)); });
    if (const auto map = Map::GetMap(place); map.has_value()) {
        return map.value();
    } else {
        throw std::runtime_error{"This map is invalid."};
    }
}

SolverSettings SimulatorBase::getSolverSettings() const {
    SolverSettings settings = m_applicationSettings.solver;
    if (settings.wind.type == WindModelType::Real) {
        settings.wind.measuredProfile = MeasuredWindProfileReader::Read(
            std::filesystem::path("input/wind") / m_applicationSettings.measuredWindFilename);
    }
    return settings;
}
