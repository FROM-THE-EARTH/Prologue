#pragma once

#include <cstddef>
#include <string>

#include "core/SimulationSettings.hpp"

struct ProcessingSettings {
    bool multiThread = false;
    size_t threadCount = 1;
};

struct ScatterSettings {
    double windSpeedMin = 0.0;
    double windSpeedMax = 0.0;
    double windDirectionInterval = 0.0;
};

struct ResultSettings {
    int precision = 8;
};

struct ApplicationSettings {
    ProcessingSettings processing;
    ScatterSettings scatter;
    ResultSettings result;
    SolverSettings solver;
    std::string measuredWindFilename;
};
