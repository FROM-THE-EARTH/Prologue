// ------------------------------------------------
// 大気モデルの設定
// ------------------------------------------------

#pragma once

struct AtmosphereSettings {
    double basePressure    = 101325.0;
    double baseTemperature = 15.0;
};

void ValidateAtmosphereSettings(const AtmosphereSettings& settings);
