// ------------------------------------------------
// SimulationRunner.hppの実装
// ------------------------------------------------

#include "SimulationRunner.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <future>
#include <mutex>
#include <stdexcept>
#include <utility>

#include "core/Simulation.hpp"

namespace {
    struct WindCondition {
        double speed;
        double direction;
    };

    void ValidateScatterSettings(const ScatterRunSettings& scatter,
                                 const SimulationExecutionSettings& execution) {
        if (!std::isfinite(scatter.windSpeedMin) || !std::isfinite(scatter.windSpeedMax)
            || scatter.windSpeedMin < 0.0 || scatter.windSpeedMax < scatter.windSpeedMin) {
            throw std::invalid_argument{
                "Scatter wind speed range must be finite, non-negative, and ordered."};
        }
        if (!std::isfinite(scatter.windDirectionInterval) || scatter.windDirectionInterval <= 0.0) {
            throw std::invalid_argument{"Scatter wind direction interval must be finite and positive."};
        }
        if (execution.multiThread && execution.threadCount == 0) {
            throw std::invalid_argument{"Simulation thread count must be greater than zero."};
        }
    }

    std::vector<WindCondition> MakeWindConditions(const ScatterRunSettings& settings) {
        std::vector<WindCondition> conditions;

        double windSpeed = settings.windSpeedMin;
        while (true) {
            double windDirection = 0.0;
            while (windDirection < 360.0) {
                conditions.push_back({.speed = windSpeed, .direction = windDirection});
                windDirection += settings.windDirectionInterval;
            }

            if (windSpeed >= settings.windSpeedMax) break;
            windSpeed += 1.0;
        }

        return conditions;
    }

    SimulationResult RunAt(const SimulationInput& baseInput, const WindCondition& wind) {
        SimulationInput input = baseInput;
        input.run.windSpeed = wind.speed;
        input.run.windDirection = {
            .degrees = wind.direction + input.environment.launchRail.azimuth,
            .reference = DirectionReference::MagneticNorth,
        };
        return Simulation::LandingPointsOnly(Simulation::Run(input));
    }
}

namespace SimulationRunner {
    SimulationResult RunDetail(const SimulationInput& input) {
        return Simulation::Run(input);
    }

    std::vector<SimulationResult> RunScatter(
        const SimulationInput& baseInput,
        const ScatterRunSettings& scatterSettings,
        const SimulationExecutionSettings& executionSettings,
        SimulationProgressObserver* progressObserver) {
        ValidateScatterSettings(scatterSettings, executionSettings);
        const auto conditions = MakeWindConditions(scatterSettings);
        std::vector<SimulationResult> results(conditions.size());

        if (progressObserver) {
            progressObserver->onStarted(conditions.size());
        }

        if (!executionSettings.multiThread || conditions.size() == 1) {
            for (size_t i = 0; i < conditions.size(); i++) {
                results[i] = RunAt(baseInput, conditions[i]);
                if (progressObserver) {
                    progressObserver->onProgress(i + 1, conditions.size());
                }
            }
            return results;
        }

        const size_t workerCount = std::min(executionSettings.threadCount, conditions.size());
        std::atomic_size_t nextIndex = 0;
        size_t completedCount = 0;
        std::mutex observerMutex;
        std::vector<std::future<void>> workers;
        workers.reserve(workerCount);

        for (size_t workerIndex = 0; workerIndex < workerCount; workerIndex++) {
            workers.emplace_back(std::async(std::launch::async, [&] {
                while (true) {
                    const size_t resultIndex = nextIndex.fetch_add(1);
                    if (resultIndex >= conditions.size()) break;

                    results[resultIndex] = RunAt(baseInput, conditions[resultIndex]);
                    {
                        const std::scoped_lock lock(observerMutex);
                        ++completedCount;
                        if (progressObserver) {
                            progressObserver->onProgress(completedCount, conditions.size());
                        }
                    }
                }
            }));
        }

        for (auto& worker : workers) {
            worker.get();
        }

        return results;
    }
}
