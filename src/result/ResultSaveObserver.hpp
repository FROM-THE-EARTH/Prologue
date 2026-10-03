// ------------------------------------------------
// Result export progress observer
// ------------------------------------------------

#pragma once

#include <cstddef>

class ResultSaveObserver {
public:
    virtual ~ResultSaveObserver() = default;

    virtual void onBodyStarted(size_t bodyIndex, size_t rowCount) = 0;
    virtual void onBodyProgress(size_t bodyIndex,
                               size_t completedRowCount,
                               size_t totalRowCount) = 0;
};
