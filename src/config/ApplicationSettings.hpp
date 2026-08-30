#pragma once

#include <cstddef>
#include <string>

#include "core/SimulationSettings.hpp"
#include "runner/SimulationRunSettings.hpp"

struct ResultSettings {
    int precision = 8;
};

struct ApplicationSettings {
    SimulationExecutionSettings execution;
    ScatterRunSettings scatter;
    ResultSettings result;
    SolverSettings solver;
    std::string measuredWindFilename;
};
