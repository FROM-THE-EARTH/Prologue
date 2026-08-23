// ------------------------------------------------
// 結果保存用関数
// ------------------------------------------------

#pragma once

#include <string>
#include <vector>

#include "core/SimulationResult.hpp"
#include "env/Map.hpp"

namespace ResultSaver {
    void SaveScatter(const std::string& dir,
                     const std::vector<SimulationResult>& result,
                     const MapData& map,
                     int precision);

    void SaveDetail(const std::string& dir,
                    const SimulationResult& result,
                    const MapData& map,
                    int precision);
}
