// ------------------------------------------------
// ScatterSimulator.hppの実装
// ------------------------------------------------

#include "ScatterSimulator.hpp"

#include <boost/progress.hpp>

#include "core/Simulation.hpp"
#include "result/ResultSaver.hpp"

template <typename T>
bool isFutureReady(const std::future<T>& f) {
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

bool ScatterSimulator::simulate() {
    m_windSpeed     = m_applicationSettings.scatter.windSpeedMin;
    m_windDirection = 0.0;

    bool complete = false;

    if (m_applicationSettings.processing.multiThread) {
        complete = multiThreadSimulation();
    } else {
        complete = singleThreadSimulation();
    }

    if (!complete) {
        return false;
    }

    return true;
}

SimulationResult ScatterSimulator::solve(double windSpeed, double windDir) {
    const double appliedWindDirection = windDir + m_input.environment.railAzimuth;
    SimulationInput input = m_input;
    input.run.windSpeed = windSpeed;
    input.run.windDirection = {
        .degrees = appliedWindDirection,
        .reference = DirectionReference::MagneticNorth,
    };

    return Simulation::LandingPointsOnly(Simulation::Run(input));
}

bool ScatterSimulator::singleThreadSimulation() {
    const size_t simulationCount =
        static_cast<size_t>(std::ceil(360 / m_applicationSettings.scatter.windDirectionInterval)
                            * (m_applicationSettings.scatter.windSpeedMax
                               - m_applicationSettings.scatter.windSpeedMin + 1));
    boost::progress_display pd(static_cast<uint32_t>(simulationCount));

    while (1) {
        m_result.emplace_back(solve(m_windSpeed, m_windDirection));
        ++pd;

        if (!updateWindCondition()) {
            break;
        }
    }

    return true;
}

bool ScatterSimulator::launchNextAsyncSolve(AsyncSolver& solver) {
    solver = std::async(std::launch::async, &ScatterSimulator::solve, this, m_windSpeed, m_windDirection);
    return updateWindCondition();
}

bool ScatterSimulator::multiThreadSimulation() {
    const size_t simulationCount =
        static_cast<size_t>(std::ceil(360 / m_applicationSettings.scatter.windDirectionInterval)
                            * (m_applicationSettings.scatter.windSpeedMax
                               - m_applicationSettings.scatter.windSpeedMin + 1));

    bool simulationFinished = false;
    size_t indexCounter     = 0;

    std::vector<AsyncSolver> solvers(m_applicationSettings.processing.threadCount);
    std::vector<size_t> threadTargetIndexes(m_applicationSettings.processing.threadCount);

    m_result.resize(simulationCount);

    boost::progress_display pd(static_cast<uint32_t>(simulationCount));

    // Launch initial solves
    for (size_t i = 0; i < m_applicationSettings.processing.threadCount; i++) {
        if (!simulationFinished) {
            simulationFinished     = !launchNextAsyncSolve(solvers[i]);
            threadTargetIndexes[i] = indexCounter++;
        }
    }

    while (true) {
        if (!simulationFinished) {
            for (size_t i = 0; i < m_applicationSettings.processing.threadCount; i++) {
                // If solvers[i].thread is ready to get the result, get it and solve next
                if (!simulationFinished && isFutureReady(solvers[i])) {
                    m_result[threadTargetIndexes[i]] = solvers[i].get();
                    simulationFinished               = !launchNextAsyncSolve(solvers[i]);
                    threadTargetIndexes[i]           = indexCounter++;
                    ++pd;
                }
            }
        }
        // Get results and end simulation
        else {
            for (size_t i = 0; i < m_applicationSettings.processing.threadCount; i++) {
                // Wait for simulations to finish and get results
                m_result[threadTargetIndexes[i]] = solvers[i].get();
                ++pd;
            }
            break;
        }
    }

    return true;
}

void ScatterSimulator::saveResult() {
    const std::string dir = "result/" + m_outputDirName + "/";
    ResultSaver::SaveScatter(dir, m_result, m_mapData, m_applicationSettings.result.precision);
}

bool ScatterSimulator::updateWindCondition() {
    m_windDirection += m_applicationSettings.scatter.windDirectionInterval;
    if (m_windDirection >= 360.0) {
        if (m_windSpeed >= m_applicationSettings.scatter.windSpeedMax) {
            return false;
        }
        m_windDirection = 0.0;
        m_windSpeed += 1.0;
    }

    return true;
}
