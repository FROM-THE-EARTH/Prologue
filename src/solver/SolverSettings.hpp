// ------------------------------------------------
// Solverの設定
// ------------------------------------------------

#pragma once

#include "dynamics/AtmosphereSettings.hpp"
#include "dynamics/WindModelSettings.hpp"

struct SolverSettings {
    double timeStep = 0.001;
    unsigned int resultStepSaveInterval = 10;
    AtmosphereSettings atmosphere;
    WindModelSettings wind;
};

void ValidateSolverSettings(const SolverSettings& settings);
