#pragma once

#include "core/SimulationInput.hpp"
#include "core/SimulationResult.hpp"

namespace Simulation {
    [[nodiscard]] SimulationResult Run(const SimulationInput& input);

    [[nodiscard]] SimulationResult LandingPointsOnly(SimulationResult result);
}
