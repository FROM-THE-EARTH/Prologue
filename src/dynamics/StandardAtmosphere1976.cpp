#include "StandardAtmosphere1976.hpp"

#include <array>
#include <cmath>
#include <stdexcept>
#include <string>

namespace StandardAtmosphere1976 {
    namespace {
        // U.S. Standard Atmosphere, 1976, Part 1, sections 3 and 4.
        constexpr double EffectiveEarthRadius = 6356766.0;  // [m]
        constexpr double StandardGravity = 9.80665;         // [m/s^2]
        constexpr double MolarGasConstant = 8314.32;        // [J/(kmol K)]
        constexpr double SeaLevelMolarMass = 28.9644;       // [kg/kmol]
        constexpr double SpecificGasConstant = MolarGasConstant / SeaLevelMolarMass;
        constexpr double CelsiusOffset = 273.15;

        struct Layer {
            double baseHeight;  // Geopotential height [m]
            double lapseRate;   // [K/m]
        };

        constexpr std::array<Layer, 3> Layers = {{
            {.baseHeight = 0.0, .lapseRate = -6.5e-3},
            {.baseHeight = 11000.0, .lapseRate = 0.0},
            {.baseHeight = 20000.0, .lapseRate = 1.0e-3},
        }};

        double pressureAt(double basePressure,
                          double baseTemperature,
                          double lapseRate,
                          double heightDelta) {
            if (lapseRate == 0.0) {
                return basePressure
                       * std::exp(-StandardGravity * heightDelta / (SpecificGasConstant * baseTemperature));
            }

            const double temperature = baseTemperature + lapseRate * heightDelta;
            return basePressure
                   * std::pow(temperature / baseTemperature,
                              -StandardGravity / (SpecificGasConstant * lapseRate));
        }
    }

    double toGeopotentialHeight(double geometricHeight) {
        if (!std::isfinite(geometricHeight) || geometricHeight <= -EffectiveEarthRadius) {
            throw std::invalid_argument{"Geometric height must be finite and greater than -6356766 m."};
        }

        return EffectiveEarthRadius * geometricHeight / (EffectiveEarthRadius + geometricHeight);
    }

    State calculate(double geometricHeight, double seaLevelPressure, double seaLevelTemperature) {
        if (!std::isfinite(seaLevelPressure) || seaLevelPressure <= 0.0) {
            throw std::invalid_argument{"Sea-level pressure must be finite and positive."};
        }
        const double seaLevelTemperatureKelvin = seaLevelTemperature + CelsiusOffset;
        const double tropopauseTemperature =
            seaLevelTemperatureKelvin + Layers[0].lapseRate * Layers[1].baseHeight;
        if (!std::isfinite(seaLevelTemperature) || tropopauseTemperature <= 0.0) {
            throw std::invalid_argument{
                "Sea-level temperature must be finite and keep every supported layer above absolute zero."};
        }

        double geopotentialHeight = toGeopotentialHeight(geometricHeight);
        if (geopotentialHeight > MaximumGeopotentialHeight + 1e-7) {
            throw std::out_of_range{"Current geopotential height is " + std::to_string(geopotentialHeight)
                                    + " m. The atmosphere model is not defined above 32000 m."};
        }
        if (geopotentialHeight > MaximumGeopotentialHeight) {
            geopotentialHeight = MaximumGeopotentialHeight;
        }

        std::array<double, Layers.size()> baseTemperatures{};
        std::array<double, Layers.size()> basePressures{};
        baseTemperatures[0] = seaLevelTemperatureKelvin;
        basePressures[0]    = seaLevelPressure;

        for (size_t i = 1; i < Layers.size(); ++i) {
            const double heightDelta = Layers[i].baseHeight - Layers[i - 1].baseHeight;
            baseTemperatures[i] =
                baseTemperatures[i - 1] + Layers[i - 1].lapseRate * heightDelta;
            basePressures[i] = pressureAt(basePressures[i - 1],
                                          baseTemperatures[i - 1],
                                          Layers[i - 1].lapseRate,
                                          heightDelta);
        }

        size_t layerIndex = 0;
        for (size_t i = 1; i < Layers.size(); ++i) {
            if (geopotentialHeight >= Layers[i].baseHeight) {
                layerIndex = i;
            }
        }

        const auto& layer = Layers[layerIndex];
        const double heightDelta       = geopotentialHeight - layer.baseHeight;
        const double temperatureKelvin = baseTemperatures[layerIndex] + layer.lapseRate * heightDelta;
        if (temperatureKelvin <= 0.0) {
            throw std::domain_error{"Calculated atmospheric temperature is not physically valid."};
        }

        const double pressure = pressureAt(basePressures[layerIndex],
                                           baseTemperatures[layerIndex],
                                           layer.lapseRate,
                                           heightDelta);
        const double radiusRatio = EffectiveEarthRadius / (EffectiveEarthRadius + geometricHeight);

        return {
            .geopotentialHeight = geopotentialHeight,
            .gravity            = StandardGravity * radiusRatio * radiusRatio,
            .temperature        = temperatureKelvin - CelsiusOffset,
            .pressure           = pressure,
            .density            = pressure / (SpecificGasConstant * temperatureKelvin),
        };
    }
}
