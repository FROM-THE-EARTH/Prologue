// ------------------------------------------------
// CLI上でシミュレーション進捗を表示するオブザーバ
// (SimulationProgressObserverの実装)
// ------------------------------------------------

#pragma once

#include <memory>

#include "runner/SimulationRunner.hpp"

namespace cli {
    class ProgressObserver final : public SimulationProgressObserver {
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        ProgressObserver();
        ~ProgressObserver() override;

        void onStarted(size_t totalSimulationCount) override;

        void onProgress(size_t completedSimulationCount, size_t totalSimulationCount) override;
    };
}
