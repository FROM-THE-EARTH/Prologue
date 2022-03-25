#include "Simulator.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "env/Map.hpp"
#include "rocket/RocketSpec.hpp"
#include "utils/ResultSaver.hpp"

bool Simulator::run() {
    if (!initialize()) {
        CommandLine::PrintInfo(PrintInfoType::Error, "Could not initialize Simulator");
        return false;
    }

    createResultDirectory();

    {
        auto place = m_rocketSpec.env.place;
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

    const auto start = std::chrono::system_clock::now();

    switch (m_simulationMode) {
    case SimulationMode::Scatter:
        scatterSimulation();
        break;

    case SimulationMode::Detail:
        detailSimulation();
        break;
    }

    if (!m_solved) {
        CommandLine::PrintInfo(PrintInfoType::Error, "Failed to simulate");
        return false;
    }

    const auto end      = std::chrono::system_clock::now();
    const auto elapsed  = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    const std::string s = "Finish processing: " + std::to_string(elapsed / 1000000.0) + "[s]";
    CommandLine::PrintInfo(PrintInfoType::Information, s.c_str());

    saveResult();

    const std::string str = "Result is saved in \"" + m_outputDirName + "/\"";
    CommandLine::PrintInfo(PrintInfoType::Information, str.c_str());

    if (m_simulationMode == SimulationMode::Scatter) {
        for (auto& r : m_scatterResult) {
            eraseNotLandingPoint(&r);
        }
    }

    CommandLine::SetOutputDir(m_outputDirName);

    return true;
}

bool Simulator::initialize() {
    // settings on commandline
    setJSONFile();

    if (AppSetting::WindModel::type == WindModelType::Real) {
        m_simulationMode = SimulationMode::Detail;
    } else {
        setSimulationMode();
    }

    setTrajectoryMode();

    if (AppSetting::WindModel::type != WindModelType::Real && m_simulationMode == SimulationMode::Detail) {
        setWindCondition();
    }

    if (RocketSpec::IsMultipleRocket(m_jsonFilename)) {
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
    m_rocketSpec.initialize(m_jsonFilename);

    // output
    m_outputDirName = m_jsonFilename;
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

    return true;
}

void Simulator::setJSONFile() {
    std::cout << "<!===Set JSON File===!>" << std::endl;
    std::vector<std::string> existJSONs;
    for (const std::filesystem::directory_entry& x : std::filesystem::directory_iterator("input/json")) {
        existJSONs.push_back(x.path().filename().string());
    }
    for (size_t i = 0; i < existJSONs.size(); i++) {
        std::cout << i + 1 << ": " << existJSONs[i] << std::endl;
    }
    const size_t jsonIndex = CommandLine::InputIndex<size_t>(existJSONs.size());
    m_jsonFilename         = existJSONs[jsonIndex - 1];
}

void Simulator::setSimulationMode() {
    CommandLine::Question("Set Simulation Mode", "Scatter Mode", "Detail Mode");
    m_simulationMode = CommandLine::InputIndex<SimulationMode>(2);
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

void Simulator::scatterSimulation() {
    m_windSpeed     = AppSetting::Simulation::windSpeedMin;
    m_windDirection = 0.0;

    m_solved = true;

    if (AppSetting::Processing::multiThread) {
        multiThreadSimulation();
    } else {
        singleThreadSimulation();
    }
}

void Simulator::detailSimulation() {
    Solver solver(m_dt, m_mapData, m_rocketType, m_trajectoryMode, m_detachType, m_detachTime, m_rocketSpec);

    m_solved = solver.run(m_windSpeed, m_windDirection);
    if (!m_solved) {
        return;
    }

    m_detailResult = solver.getResult();
    m_detailResult.organize(m_mapData);
}

void Simulator::singleThreadSimulation() {
    while (1) {
        Solver solver(m_dt, m_mapData, m_rocketType, m_trajectoryMode, m_detachType, m_detachTime, m_rocketSpec);

        m_solved &= solver.run(m_windSpeed, m_windDirection);
        if (!m_solved) {
            break;
        }

        auto result = solver.getResult();
        result.organize(m_mapData);

        result = formatResultForScatter(result);
        m_scatterResult.push_back(result);

        if (!updateWindCondition()) {
            break;
        }
    }
}

void Simulator::multiThreadSimulation() {
    const size_t threadCount = 2;
    bool finish              = false;
    size_t simulated         = 1;

    while (!finish) {
        bool e = false;

        SolvedResult results[threadCount];
        std::thread threads[threadCount];
        bool finished[threadCount] = {false};
        bool error[threadCount]    = {false};

        simulated = 1;

        threads[0] =
            std::thread(&Simulator::solve, this, m_windSpeed, m_windDirection, &results[0], &finished[0], &error[0]);
        threads[0].detach();
        for (size_t i = 1; i < threadCount; i++) {
            finish = !updateWindCondition();
            if (finish) {
                break;
            }

            threads[i] = std::thread(
                &Simulator::solve, this, m_windSpeed, m_windDirection, &results[i], &finished[i], &error[i]);
            threads[i].detach();
            simulated++;
        }

        // wait threads
        while (1) {
            bool f = true;
            for (size_t i = 0; i < simulated; i++) {
                f = f && (finished[i] || error[i]);
                e = e || error[i];
            }
            if (f) {
                break;
            }
            std::cout << "";
        }

        // error occured
        if (e) {
            m_solved = false;
            break;
        }

        for (size_t i = 0; i < simulated; i++) {
            m_scatterResult.push_back(results[i]);
        }

        finish = !updateWindCondition();
    }
}

void Simulator::solve(double windSpeed, double windDir, SolvedResult* result, bool* finish, bool* error) {
    Solver solver(m_dt, m_mapData, m_rocketType, m_trajectoryMode, m_detachType, m_detachTime, m_rocketSpec);

    if (*error = !solver.run(windSpeed, windDir); *error) {
        return;
    }

    *result = solver.getResult();
    result->organize(m_mapData);
    *result = formatResultForScatter(*result);
    *finish = true;
}

SolvedResult Simulator::formatResultForScatter(const SolvedResult& result) {
    SolvedResult res = result;

    for (auto& r : res.rocket) {
        Rocket landing = r.flightData[r.flightData.size() - 1];
        const auto v   = std::vector<Rocket>(1, landing);
        r.flightData   = v;
    }

    return res;
}

void Simulator::eraseNotLandingPoint(SolvedResult* result) {
    const int size = static_cast<int>(result->rocket.size());
    for (int i = size - 1; i >= 0; i--) {
        if (result->rocket[i].flightData[0].pos.z != 0.0) {
            result->rocket.erase(result->rocket.begin() + i);
        }
    }
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

void Simulator::saveResult() {
    const std::string dir = "result/" + m_outputDirName + "/";

    switch (m_simulationMode) {
    case SimulationMode::Scatter:
        ResultSaver::SaveScatter(dir, m_scatterResult);
        break;

    case SimulationMode::Detail:
        ResultSaver::SaveDetail(dir, m_detailResult);
        ResultSaver::SaveDetailAll(dir, m_detailResult);
        break;
    }
}

bool Simulator::updateWindCondition() {
    m_windDirection += AppSetting::Simulation::windDirInterval;
    if (m_windDirection >= 360.0) {
        m_windDirection = 0.0;
        m_windSpeed += 1.0;
        if (m_windSpeed > AppSetting::Simulation::windSpeedMax) {
            return false;
        }
    }

    return true;
}
