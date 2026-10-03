#pragma once

#include <vector>

#include "core/SimulationResult.hpp"
#include "env/Environment.hpp"
#include "runner/SimulationRunSettings.hpp"

namespace gui {
    // Immutable data shared by the plot and map views after a run completes.
    struct RunResults {
        Environment environment;
        SimulationMode mode = SimulationMode::Detail;
        std::vector<SimulationResult> simulations;
    };
}
