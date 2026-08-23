#pragma once

#include <vector>

#include "core/SimulationResult.hpp"
#include "env/Map.hpp"

struct GeographicPosition {
    double latitude = 0.0;
    double longitude = 0.0;
};

struct GeographicResult {
    std::vector<std::vector<GeographicPosition>> bodyStepPositions;
    std::vector<GeographicPosition> bodyFinalPositions;
};

namespace GeographicResultAdapter {
    [[nodiscard]] GeographicResult Convert(const SimulationResult& result, const MapData& map);
}
