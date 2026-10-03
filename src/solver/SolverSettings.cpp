// ------------------------------------------------
// SolverSettings.hppの実装
// ------------------------------------------------

#include "SolverSettings.hpp"

#include <cmath>
#include <stdexcept>

void ValidateSolverSettings(const SolverSettings& settings) {
    if (!std::isfinite(settings.timeStep) || settings.timeStep <= 0.0) {
        throw std::invalid_argument{"Solver time step must be finite and positive."};
    }
    if (settings.resultStepSaveInterval == 0) {
        throw std::invalid_argument{"Result step save interval must be greater than zero."};
    }

    ValidateAtmosphereSettings(settings.atmosphere);
    ValidateWindModelSettings(settings.wind);
}
