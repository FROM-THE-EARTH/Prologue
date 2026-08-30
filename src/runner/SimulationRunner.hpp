// ------------------------------------------------
// 単一条件および散布条件でのシミュレーション実行
// ------------------------------------------------

#pragma once

#include <cstddef>
#include <vector>

#include "core/SimulationInput.hpp"
#include "core/SimulationResult.hpp"
#include "runner/SimulationRunSettings.hpp"

class SimulationProgressObserver {
public:
    virtual ~SimulationProgressObserver() = default;

    virtual void onStarted(size_t totalSimulationCount) = 0;

    // Calls are serialized. During parallel execution they may originate from a worker thread.
    virtual void onProgress(size_t completedSimulationCount, size_t totalSimulationCount) = 0;
};

namespace SimulationRunner {
    [[nodiscard]] SimulationResult RunDetail(const SimulationInput& input);

    [[nodiscard]] std::vector<SimulationResult> RunScatter(
        const SimulationInput& baseInput,
        const ScatterRunSettings& scatterSettings,
        const SimulationExecutionSettings& executionSettings,
        SimulationProgressObserver* progressObserver = nullptr);
}
