// ------------------------------------------------
// 風モデルの設定
// ------------------------------------------------

#pragma once

#include <optional>

#include "dynamics/WindProfile.hpp"

enum class WindModelType { Real, Original, OnlyPowerLow, NoWind };

struct WindModelSettings {
    WindModelType type = WindModelType::NoWind;
    double powerConstant = 6.0;
    double powerLowBaseAltitude = 2.0;
    std::optional<WindProfile> measuredProfile;
};

void ValidateWindModelSettings(const WindModelSettings& settings);
