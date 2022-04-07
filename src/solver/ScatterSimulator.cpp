#include "ScatterSimulator.hpp"

#include <thread>

#include "app/AppSetting.hpp"
#include "utils/ResultSaver.hpp"

bool ScatterSimulator::simulate() {
    m_windSpeed     = AppSetting::Simulation::windSpeedMin;
    m_windDirection = 0.0;

    bool complete = false;

    if (AppSetting::Processing::multiThread) {
        complete = multiThreadSimulation();
    } else {
        complete = singleThreadSimulation();
    }

    if (!complete) {
        return false;
    }

    return true;
}

void ScatterSimulator::solve(double windSpeed, double windDir, SolvedResult* result, bool* finish, bool* error) {
    Solver solver(
        m_dt, m_mapData, m_rocketType, m_trajectoryMode, m_detachType, m_detachTime, m_environment, m_rocketSpec);

    if (*error = !solver.run(windSpeed, windDir); *error) {
        return;
    }

    *result = solver.getResult();
    result->organize(m_mapData);
    *result = formatResultForScatter(*result);
    *finish = true;
}

bool ScatterSimulator::singleThreadSimulation() {
    while (1) {
        Solver solver(
            m_dt, m_mapData, m_rocketType, m_trajectoryMode, m_detachType, m_detachTime, m_environment, m_rocketSpec);

        if (!solver.run(m_windSpeed, m_windDirection)) {
            return false;
        }

        auto result = solver.getResult();
        result.organize(m_mapData);

        result = formatResultForScatter(result);
        m_result.push_back(result);

        if (!updateWindCondition()) {
            break;
        }
    }

    return true;
}

bool ScatterSimulator::multiThreadSimulation() {
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

        threads[0] = std::thread(
            &ScatterSimulator::solve, this, m_windSpeed, m_windDirection, &results[0], &finished[0], &error[0]);
        threads[0].detach();
        for (size_t i = 1; i < threadCount; i++) {
            finish = !updateWindCondition();
            if (finish) {
                break;
            }

            threads[i] = std::thread(
                &ScatterSimulator::solve, this, m_windSpeed, m_windDirection, &results[i], &finished[i], &error[i]);
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
            return false;
        }

        for (size_t i = 0; i < simulated; i++) {
            m_result.push_back(results[i]);
        }

        finish = !updateWindCondition();
    }

    return true;
}

void ScatterSimulator::saveResult() {
    const std::string dir = "result/" + m_outputDirName + "/";
    ResultSaver::SaveScatter(dir, m_result);
}

void ScatterSimulator::plotToGnuplot() {
    auto resultForPlot = m_result;
    for (auto& r : resultForPlot) {
        eraseNotLandingPoint(&r);
    }
    Gnuplot::Plot(resultForPlot);
}

SolvedResult ScatterSimulator::formatResultForScatter(const SolvedResult& result) {
    SolvedResult res = result;

    for (auto& rocket : res.rockets) {
        const Body landedBody = rocket.flightData[rocket.flightData.size() - 1];
        rocket.flightData     = std::vector<Body>(1, landedBody);
    }

    return res;
}

void ScatterSimulator::eraseNotLandingPoint(SolvedResult* result) {
    const int size = static_cast<int>(result->rockets.size());
    for (int i = size - 1; i >= 0; i--) {
        if (result->rockets[i].flightData[0].pos.z != 0.0) {
            result->rockets.erase(result->rockets.begin() + i);
        }
    }
}

bool ScatterSimulator::updateWindCondition() {
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
