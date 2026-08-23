#include "GeographicResult.hpp"

namespace GeographicResultAdapter {
    GeographicResult Convert(const SimulationResult& result, const MapData& map) {
        GeographicResult geographic;
        geographic.bodyStepPositions.reserve(result.bodyResults.size());

        for (const auto& bodyResult : result.bodyResults) {
            auto& positions = geographic.bodyStepPositions.emplace_back();
            positions.reserve(bodyResult.steps.size());
            for (const auto& step : bodyResult.steps) {
                const auto [latitude, longitude] =
                    map.coordinate.LatLonAt(step.rocket_pos.x, step.rocket_pos.y);
                positions.push_back({.latitude = latitude, .longitude = longitude});
            }
        }

        geographic.bodyFinalPositions.reserve(result.bodyFinalPositions.size());
        for (const auto& position : result.bodyFinalPositions) {
            const auto [latitude, longitude] = map.coordinate.LatLonAt(position.x, position.y);
            geographic.bodyFinalPositions.push_back(
                {.latitude = latitude, .longitude = longitude});
        }

        return geographic;
    }
}
