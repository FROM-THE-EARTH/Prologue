#include "SimulationSettings.hpp"

#include <cmath>
#include <stdexcept>

#include "dynamics/StandardAtmosphere1976.hpp"

void ValidateAtmosphereSettings(const AtmosphereSettings& settings) {
    // Reuse the atmosphere model's own domain checks so validation and
    // calculation cannot drift apart.
    static_cast<void>(StandardAtmosphere1976::calculate(
        0.0, settings.basePressure, settings.baseTemperature));
}

void ValidateWindModelSettings(const WindModelSettings& settings) {
    if ((settings.type == WindModelType::Original || settings.type == WindModelType::OnlyPowerLow)
        && (!std::isfinite(settings.powerConstant) || settings.powerConstant <= 0.0)) {
        throw std::invalid_argument{"Wind power constant must be finite and positive."};
    }
    if ((settings.type == WindModelType::Original || settings.type == WindModelType::OnlyPowerLow)
        && (!std::isfinite(settings.powerLowBaseAltitude) || settings.powerLowBaseAltitude <= 0.0)) {
        throw std::invalid_argument{"Wind power-law base altitude must be finite and positive."};
    }
    if (settings.type == WindModelType::Real && !settings.measuredProfile.has_value()) {
        throw std::invalid_argument{"Measured wind model requires a wind profile."};
    }
}

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
