// ------------------------------------------------
// WindModel.hppの実装
// ------------------------------------------------

#include "WindModel.hpp"

#include "StandardAtmosphere1976.hpp"

#include <cmath>
#include <stdexcept>

#include "misc/Constant.hpp"

namespace Atmospehre {
    namespace Wind {
        constexpr double GeostrophicWind = 15;  // 地衡風 [m/s]

        constexpr double SurfaceLayerLimit = 300;  // 接地境界層 0 ~ 300 [m]

        // --------------------------------------------------------------
        // エクマン層
        // Wikipedia: https://ja.wikipedia.org/wiki/%E3%82%A8%E3%82%AF%E3%83%9E%E3%83%B3%E5%A2%83%E7%95%8C%E5%B1%A4
        // From: http://kishou.u-gakugei.ac.jp/graduate/local/doc04.pdf
        // --------------------------------------------------------------
        constexpr double EkmanLayerLimit = 1000;  // エクマン層 300 ~ 1000 [m]
    }
}

// 0未満または360以上の角度を0 ~ 360におさめる
double normalizeAngle(double angle) {
    if (0 <= angle && angle < 360) {
        return angle;
    } else if (-360 <= angle && angle < 0) {
        return 360 + angle;
    } else if (angle >= 360) {
        return angle - 360 * static_cast<int>(std::floor(angle / 360));
    } else if (angle < -360) {
        return angle - 360 * static_cast<int>(std::floor(angle / 360));
    } else {
        return angle;
    }
}

double applyPowerLow(double windSpeed, double height, const WindModelSettings& settings) {
    return windSpeed * pow(height / settings.powerLowBaseAltitude, 1.0 / settings.powerConstant);
}

Vector3D applyPowerLow(const Vector3D& wind, double height, const WindModelSettings& settings) {
    return wind * pow(height / settings.powerLowBaseAltitude, 1.0 / settings.powerConstant);
}

WindModel::WindModel(const WindModelSettings& settings,
                     const AtmosphereSettings& atmosphereSettings,
                     double groundWindSpeed,
                     const WindDirection& groundWindDirection,
                     double magneticDeclination) :
    m_settings(settings),
    m_atmosphereSettings(atmosphereSettings),
    m_groundWindSpeed(groundWindSpeed),
    m_groundWindDirection(normalizeAngle(ResolveTrueNorthDirection(groundWindDirection, magneticDeclination))),
    m_magneticDeclination(magneticDeclination) {
    ValidateAtmosphereSettings(m_atmosphereSettings);
    ValidateWindModelSettings(m_settings);
    if (!std::isfinite(m_groundWindSpeed) || m_groundWindSpeed < 0.0) {
        throw std::invalid_argument{"Ground wind speed must be finite and non-negative."};
    }
    m_directionInterval = 270 - m_groundWindDirection;
    if (m_directionInterval <= -45.0) {
        m_directionInterval = 270 - m_groundWindDirection + 360;
    }
}

WindModel::AtmosphericConditions WindModel::sampleAt(double height) const {
    const auto atmosphere = StandardAtmosphere1976::calculate(
        height, m_atmosphereSettings.basePressure, m_atmosphereSettings.baseTemperature);

    Vector3D wind;

    switch (m_settings.type) {
    case WindModelType::Real:
        wind = getWindFromData(height);
        break;

    case WindModelType::Original:
        wind = getWindOriginalModel(height);
        break;

    case WindModelType::OnlyPowerLow:
        wind = getWindOnlyPowerLow(height);
        break;
    case WindModelType::NoWind:
        wind = Vector3D(0, 0, 0);
        break;
    }

    return {.wind        = wind,
            .density     = atmosphere.density,
            .gravity     = atmosphere.gravity,
            .pressure    = atmosphere.pressure,
            .temperature = atmosphere.temperature};
}

Vector3D WindModel::getWindFromData(double height) const {
    return m_settings.measuredProfile->windAt(height, m_magneticDeclination);
}

Vector3D WindModel::getWindOriginalModel(double height) const {
    if (height < 0) {
        return Vector3D();
    }

    if (height < Atmospehre::Wind::SurfaceLayerLimit) {  // Surface layer
        const double deltaDirection = height / Atmospehre::Wind::EkmanLayerLimit * m_directionInterval;
        const double rad            = (m_groundWindDirection + deltaDirection) * Constant::PI / 180;
        const Vector3D wind         = -Vector3D(sin(rad), cos(rad), 0) * m_groundWindSpeed;

        return applyPowerLow(wind, height, m_settings);
    } else if (height < Atmospehre::Wind::EkmanLayerLimit) {  // Ekman layer
        const double deltaDirection = height / Atmospehre::Wind::EkmanLayerLimit * m_directionInterval;
        const double rad            = (m_groundWindDirection + deltaDirection) * Constant::PI / 180;

        const double borderWindSpeed = applyPowerLow(m_groundWindSpeed, height, m_settings);

        const double k =
            (height - Atmospehre::Wind::SurfaceLayerLimit) / (Atmospehre::Wind::SurfaceLayerLimit * sqrt(2));
        const double u = Atmospehre::Wind::GeostrophicWind * (1 - exp(-k) * cos(k));
        const double v = Atmospehre::Wind::GeostrophicWind * exp(-k) * sin(k);

        const double descentRate = ((Atmospehre::Wind::GeostrophicWind - u) / Atmospehre::Wind::GeostrophicWind);

        const Vector3D ekmanWind(u, v, 0);
        return -Vector3D(sin(rad), cos(rad), 0) * borderWindSpeed * descentRate + ekmanWind;
    } else {  // Free atomosphere
        return Vector3D(Atmospehre::Wind::GeostrophicWind, 0, 0);
    }
}

Vector3D WindModel::getWindOnlyPowerLow(double height) const {
    if (height < 0) {
        return Vector3D();
    } else {
        const double rad    = m_groundWindDirection * Constant::PI / 180;
        const Vector3D wind = -Vector3D(sin(rad), cos(rad), 0) * m_groundWindSpeed;

        return applyPowerLow(wind, height, m_settings);
    }
}
