#include "ScatterSimulator.hpp"

#include <thread>

#include "app/AppSetting.hpp"
#include "result/ResultSaver.hpp"

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

void ScatterSimulator::solve(
    double windSpeed, double windDir, std::shared_ptr<SimuResultLogger>& resultLogger, bool* finish, bool* error) {
    Solver solver(
        m_dt, m_mapData, m_rocketType, m_trajectoryMode, m_detachType, m_detachTime, m_environment, m_rocketSpec);

    if (resultLogger = solver.solve(windSpeed, windDir); resultLogger) {
        resultLogger->organize();
        *error = false;
    } else {
        *error = true;
    }

    *finish = true;
}

bool ScatterSimulator::singleThreadSimulation() {
    while (1) {
        Solver solver(
            m_dt, m_mapData, m_rocketType, m_trajectoryMode, m_detachType, m_detachTime, m_environment, m_rocketSpec);

        if (const auto result = solver.solve(m_windSpeed, m_windDirection); result) {
            result->organize();
            m_result.push_back(result->getResultScatterFormat());
        } else {
            return false;
        }

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

        std::shared_ptr<SimuResultLogger> results[threadCount];
        std::thread threads[threadCount];
        bool finished[threadCount] = {false};
        bool error[threadCount]    = {false};

        simulated = 1;

        threads[0] = std::thread(&ScatterSimulator::solve,
                                 this,
                                 m_windSpeed,
                                 m_windDirection,
                                 std::ref(results[0]),
                                 &finished[0],
                                 &error[0]);
        threads[0].detach();
        for (size_t i = 1; i < threadCount; i++) {
            finish = !updateWindCondition();
            if (finish) {
                break;
            }

            threads[i] = std::thread(&ScatterSimulator::solve,
                                     this,
                                     m_windSpeed,
                                     m_windDirection,
                                     std::ref(results[i]),
                                     &finished[i],
                                     &error[i]);
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
            m_result.emplace_back(results[i]->getResultScatterFormat());
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
    Gnuplot::Plot(m_result);
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
