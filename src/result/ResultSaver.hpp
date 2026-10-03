// ------------------------------------------------
// 結果保存用関数
// ------------------------------------------------

#pragma once

#include <filesystem>
#include <vector>

#include "core/SimulationResult.hpp"
#include "env/Environment.hpp"
#include "result/ResultSaveObserver.hpp"

namespace ResultSaver {
    void SaveScatter(const std::filesystem::path& dir,
                     const std::vector<SimulationResult>& result,
                     const Environment& environment,
                     int precision,
                     ResultSaveObserver* observer = nullptr);

    void SaveDetail(const std::filesystem::path& dir,
                    const SimulationResult& result,
                    const Environment& environment,
                    int precision,
                    ResultSaveObserver* observer = nullptr);
}
