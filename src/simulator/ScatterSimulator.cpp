// ------------------------------------------------
// ScatterSimulator.hppの実装
// ------------------------------------------------

#include "ScatterSimulator.hpp"

#include <boost/progress.hpp>

#include "app/AppSetting.hpp"
#include "gnuplot/plotter/Plotter2D.hpp"
#include "result/ResultSaver.hpp"

template <typename T>
bool isFutureReady(const std::future<T>& f) {
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

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

std::shared_ptr<SimuResultLogger> ScatterSimulator::solve(double windSpeed, double windDir) {
	RocketSpecification rocektSpecCopy = m_rocketSpec; // deep copy
    Solver solver(m_mapData,
                  m_rocketType,
                  m_setting.trajectoryMode,
                  m_setting.detachType,
                  m_setting.detachTime,
                  m_environment,
                  rocektSpecCopy);

    // Wind direction is based on launch azimuth
    if (const auto result = solver.solve(windSpeed, windDir + m_environment.railAzimuth); result) {
        result->organize();
        return result;
    } else {
        return nullptr;
    }
}

bool ScatterSimulator::singleThreadSimulation() {
    const size_t simulationCount =
        static_cast<size_t>(std::ceil(360 / AppSetting::Simulation::windDirInterval)
                            * (AppSetting::Simulation::windSpeedMax - AppSetting::Simulation::windSpeedMin + 1));
    boost::progress_display pd(static_cast<uint32_t>(simulationCount));

    while (1) {
        if (const auto result = solve(m_windSpeed, m_windDirection)) {
            m_result.emplace_back(result->getResultScatterFormat());
            ++pd;
        } else {
            return false;
        }

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
        static_cast<size_t>(std::ceil(360 / AppSetting::Simulation::windDirInterval)
                            * (AppSetting::Simulation::windSpeedMax - AppSetting::Simulation::windSpeedMin + 1));

    bool simulationFinished = false;
    size_t indexCounter     = 0;

    std::vector<AsyncSolver> solvers(AppSetting::Processing::threadCount);
    std::vector<size_t> threadTargetIndexes(AppSetting::Processing::threadCount);

    m_result.resize(simulationCount);

    boost::progress_display pd(static_cast<uint32_t>(simulationCount));

    // Launch initial solves
    for (size_t i = 0; i < AppSetting::Processing::threadCount; i++) {
        if (!simulationFinished) {
            simulationFinished     = !launchNextAsyncSolve(solvers[i]);
            threadTargetIndexes[i] = indexCounter++;
        }
    }

    while (true) {
        if (!simulationFinished) {
            for (size_t i = 0; i < AppSetting::Processing::threadCount; i++) {
                // If solvers[i].thread is ready to get the result, get it and solve next
                if (!simulationFinished && isFutureReady(solvers[i])) {
                    m_result[threadTargetIndexes[i]] = solvers[i].get()->getResultScatterFormat();
                    simulationFinished               = !launchNextAsyncSolve(solvers[i]);
                    threadTargetIndexes[i]           = indexCounter++;
                    ++pd;
                }
            }
        }
        // Get results and end simulation
        else {
            for (size_t i = 0; i < AppSetting::Processing::threadCount; i++) {
                // Wait for simulations to finish and get results
                m_result[threadTargetIndexes[i]] = solvers[i].get()->getResultScatterFormat();
                ++pd;
            }
            break;
        }
    }

    return true;
}

void ScatterSimulator::saveResult() {
    const std::string dir = "result/" + m_outputDirName + "/";
    ResultSaver::SaveScatter(dir, m_result);
}

void ScatterSimulator::plotToGnuplot() {
    auto plotter = Plotter2D("result/" + m_outputDirName + "/", m_result[0].bodyResults.size(), m_mapData);
    plotter.saveResult(m_result);
    plotter.savePlot();
    plotter.savePlotAsPng();
}

bool ScatterSimulator::updateWindCondition() {
    m_windDirection += AppSetting::Simulation::windDirInterval;
    if (m_windDirection >= 360.0) {
        if (m_windSpeed >= AppSetting::Simulation::windSpeedMax) {
            return false;
        }
        m_windDirection = 0.0;
        m_windSpeed += 1.0;
    }

    return true;
}
