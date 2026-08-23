// ------------------------------------------------
// 風モデルクラス
// ------------------------------------------------

#pragma once

#include "core/SimulationSettings.hpp"
#include "dynamics/WindProfile.hpp"

class WindModel {
    const WindModelSettings m_settings;
    const AtmosphereSettings m_atmosphereSettings;

    const double m_groundWindSpeed, m_groundWindDirection;
    const double m_magneticDeclination;

    double m_directionInterval = 0.0;

public:
    struct AtmosphericConditions {
        Vector3D wind;
        double density     = 0.0;  // [kg/m^3]
        double gravity     = 0.0;  // [m/s^2]
        double pressure    = 0.0;  // [Pa]
        double temperature = 0.0;  // [°C]
    };

    explicit WindModel(const WindModelSettings& settings,
                       const AtmosphereSettings& atmosphereSettings,
                       double groundWindSpeed,
                       const WindDirection& groundWindDirection,
                       double magneticDeclination);

    // 指定した幾何高度における風と大気状態を取得
    [[nodiscard]] AtmosphericConditions sampleAt(double height) const;

private:
    Vector3D getWindFromData(double height) const;

    Vector3D getWindOriginalModel(double height) const;

    Vector3D getWindOnlyPowerLow(double height) const;
};
