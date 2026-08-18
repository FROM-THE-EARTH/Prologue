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

public:
    struct AtmosphericConditions {
        Vector3D wind;
        double density     = 0.0;  // [kg/m^3]
        double gravity     = 0.0;  // [m/s^2]
        double pressure    = 0.0;  // [Pa]
        double temperature = 0.0;  // [°C]
    };

    // 風向風速ファイルから風モデルを構築
    explicit WindModel(double magneticDeclination);

    // オリジナル、またはべき乗則での風モデル構築
    explicit WindModel(double groundWindSpeed,
                       double groundWindDirection,
                       double magneticDeclination);  // original or only_powerlow

    // 指定した幾何高度における風と大気状態を取得
    [[nodiscard]] AtmosphericConditions sampleAt(double height) const;

private:
    Vector3D getWindFromData(double height) const;

    Vector3D getWindOriginalModel(double height) const;

    Vector3D getWindOnlyPowerLow(double height) const;
};
