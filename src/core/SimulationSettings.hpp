#pragma once

#include <cstddef>
#include <optional>

#include "dynamics/WindProfile.hpp"

enum class WindModelType { Real, Original, OnlyPowerLow, NoWind };

struct AtmosphereSettings {
    double basePressure    = 101325.0;
    double baseTemperature = 15.0;
};

struct WindModelSettings {
    WindModelType type = WindModelType::NoWind;
    double powerConstant = 6.0;
    double powerLowBaseAltitude = 2.0;
    std::optional<WindProfile> measuredProfile;
};

struct SolverSettings {
    double timeStep = 0.001;
    unsigned int resultStepSaveInterval = 10;
    AtmosphereSettings atmosphere;
    WindModelSettings wind;
};

void ValidateAtmosphereSettings(const AtmosphereSettings& settings);

void ValidateWindModelSettings(const WindModelSettings& settings);

void ValidateSolverSettings(const SolverSettings& settings);
