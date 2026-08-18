// ------------------------------------------------
// 風モデルクラス
// ------------------------------------------------

#pragma once

#include <optional>
#include <string>

#include "dynamics/WindProfile.hpp"

class WindModel {
    std::optional<WindProfile> m_windProfile;

    const double m_groundWindSpeed, m_groundWindDirection;

    double m_directionInterval = 0.0;

    double m_height             = 0.0;
    double m_geopotentialHeight = 0.0;
    double m_airDensity         = 0.0;
    double m_gravity            = 0.0;
    double m_pressure           = 0.0;
    double m_temperature        = 0.0;
    Vector3D m_wind;

public:
    // 風向風速ファイルから風モデルを構築
    explicit WindModel(double magneticDeclination);

    // オリジナル、またはべき乗則での風モデル構築
    explicit WindModel(double groundWindSpeed,
                       double groundWindDirection,
                       double magneticDeclination);  // original or only_powerlow

    // 高度を更新
    void update(double height);

    Vector3D wind() const {
        return m_wind;
    }

    // [kg/m^3]
    double density() const {
        return m_airDensity;
    }

    // [m/s^2]
    double gravity() const {
        return m_gravity;
    }

    // [Pa]
    double pressure() const {
        return m_pressure;
    }

    // [°C]
    double temperature() const {
        return m_temperature;
    }

private:
    double getGeopotentialHeight();

    double getGravity();

    double getTemperature();

    double getPressure();

    double getAirDensity();

    Vector3D getWindFromData();

    Vector3D getWindOriginalModel();

    Vector3D getWindOnlyPowerLow();
};
