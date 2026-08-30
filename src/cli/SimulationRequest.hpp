// ------------------------------------------------
// CLI入力から構築されるシミュレーション要求
// ------------------------------------------------

#pragma once

#include <string>

#include "cli/Option.hpp"
#include "config/ApplicationSettings.hpp"
#include "core/SimulationInput.hpp"
#include "runner/SimulationRunner.hpp"

namespace cli {
    struct SimulationRequest {
        std::string specificationName;
        SimulationMode mode = SimulationMode::Detail;
        SimulationInput input;
    };

    SimulationRequest PrepareSimulation(const CommandLineOption::Option& option,
                                        const ApplicationSettings& applicationSettings);
}
