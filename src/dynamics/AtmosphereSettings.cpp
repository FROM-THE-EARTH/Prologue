// ------------------------------------------------
// AtmosphereSettings.hppの実装
// ------------------------------------------------

#include "AtmosphereSettings.hpp"

#include "dynamics/StandardAtmosphere1976.hpp"

void ValidateAtmosphereSettings(const AtmosphereSettings& settings) {
    // Reuse the atmosphere model's own domain checks so validation and
    // calculation cannot drift apart.
    static_cast<void>(StandardAtmosphere1976::calculate(
        0.0, settings.basePressure, settings.baseTemperature));
}
