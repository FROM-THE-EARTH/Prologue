#pragma once

#include <cstddef>
#include <string>

#include "runner/SimulationRunSettings.hpp"
#include "solver/SolverSettings.hpp"

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
