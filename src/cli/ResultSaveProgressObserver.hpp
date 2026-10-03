// ------------------------------------------------
// CLI result export progress display
// ------------------------------------------------

#pragma once

#include <memory>

#include "result/ResultSaveObserver.hpp"

namespace cli {
    class ResultSaveProgressObserver final : public ResultSaveObserver {
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        ResultSaveProgressObserver();
        ~ResultSaveProgressObserver() override;

        void onBodyStarted(size_t bodyIndex, size_t rowCount) override;
        void onBodyProgress(size_t bodyIndex,
                            size_t completedRowCount,
                            size_t totalRowCount) override;
    };
}
