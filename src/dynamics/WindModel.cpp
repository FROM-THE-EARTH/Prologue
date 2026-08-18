// ------------------------------------------------
// WindModel.hppの実装
// ------------------------------------------------

#include "WindModel.hpp"

#include "StandardAtmosphere1976.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "math/Algorithm.hpp"
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
        CommandLine::PrintInfo(PrintInfoType::Warning, "Unhandled angle " + std::to_string(angle));
        return angle;
    }
}

double applyPowerLow(double windSpeed, double height) {
    return windSpeed
           * pow(height / AppSetting::WindModel::powerLowBaseAltitude, 1.0 / AppSetting::WindModel::powerConstant);
}

Vector3D applyPowerLow(const Vector3D& wind, double height) {
    return wind * pow(height / AppSetting::WindModel::powerLowBaseAltitude, 1.0 / AppSetting::WindModel::powerConstant);
}

WindModel::WindModel(double groundWindSpeed, double groundWindDirection, double magneticDeclination) :
    m_groundWindSpeed(groundWindSpeed),
    m_groundWindDirection(normalizeAngle(groundWindDirection + magneticDeclination)) {
    m_directionInterval = 270 - m_groundWindDirection;
    if (m_directionInterval <= -45.0) {
        m_directionInterval = 270 - m_groundWindDirection + 360;
    }
}

WindModel::WindModel(double magneticDeclination) : m_groundWindSpeed(0.0), m_groundWindDirection(0.0) {
    const std::string windFilePath = "input/wind/" + AppSetting::WindModel::realdataFilename;
    std::ifstream windfile(windFilePath);
    if (!windfile.is_open()) {
        throw std::runtime_error{"Failed to open wind data file: " + windFilePath};
    }

    std::string header;
    std::getline(windfile, header);

    std::vector<WindData> windData;
    std::string line;
    size_t lineNumber = 1;
    while (std::getline(windfile, line)) {
        lineNumber++;
        if (line.empty()) {
            continue;
        }

        std::istringstream row(line);
        WindData data;
        char firstComma, secondComma;
        if (!(row >> data.geometricHeight >> firstComma >> data.speed >> secondComma >> data.direction)
            || firstComma != ',' || secondComma != ',') {
            throw std::runtime_error{"Invalid wind data at line " + std::to_string(lineNumber) + " in: "
                                     + windFilePath};
        }
        row >> std::ws;
        if (!row.eof()) {
            throw std::runtime_error{"Unexpected value at line " + std::to_string(lineNumber) + " in: "
                                     + windFilePath};
        }

        data.direction += magneticDeclination;
        windData.push_back(data);
    }

    m_windProfile.emplace(std::move(windData));
}

void WindModel::update(double height) {
    m_height = height;

    const auto atmosphere = StandardAtmosphere1976::calculate(
        height, AppSetting::Atmosphere::basePressure, AppSetting::Atmosphere::baseTemperature);
    m_gravity     = atmosphere.gravity;
    m_temperature = atmosphere.temperature;
    m_pressure    = atmosphere.pressure;
    m_airDensity  = atmosphere.density;

    switch (AppSetting::WindModel::type) {
    case WindModelType::Real:
        m_wind = getWindFromData();
        break;

    case WindModelType::Original:
        m_wind = getWindOriginalModel();
        break;

    case WindModelType::OnlyPowerLow:
        m_wind = getWindOnlyPowerLow();
        break;
    case WindModelType::NoWind:
        m_wind = Vector3D(0, 0, 0);
        break;
    }
}

Vector3D WindModel::getWindFromData() {
    return m_windProfile->windAt(m_height);
}

Vector3D WindModel::getWindOriginalModel() {
    if (m_height < 0) {
        return Vector3D();
    }

    if (m_height < Atmospehre::Wind::SurfaceLayerLimit) {  // Surface layer
        const double deltaDirection = m_height / Atmospehre::Wind::EkmanLayerLimit * m_directionInterval;
        const double rad            = (m_groundWindDirection + deltaDirection) * Constant::PI / 180;
        const Vector3D wind         = -Vector3D(sin(rad), cos(rad), 0) * m_groundWindSpeed;

        return applyPowerLow(wind, m_height);
    } else if (m_height < Atmospehre::Wind::EkmanLayerLimit) {  // Ekman layer
        const double deltaDirection = m_height / Atmospehre::Wind::EkmanLayerLimit * m_directionInterval;
        const double rad            = (m_groundWindDirection + deltaDirection) * Constant::PI / 180;

        const double borderWindSpeed = applyPowerLow(m_groundWindSpeed, m_height);

        const double k =
            (m_height - Atmospehre::Wind::SurfaceLayerLimit) / (Atmospehre::Wind::SurfaceLayerLimit * sqrt(2));
        const double u = Atmospehre::Wind::GeostrophicWind * (1 - exp(-k) * cos(k));
        const double v = Atmospehre::Wind::GeostrophicWind * exp(-k) * sin(k);

        const double descentRate = ((Atmospehre::Wind::GeostrophicWind - u) / Atmospehre::Wind::GeostrophicWind);

        return -Vector3D(sin(rad), cos(rad), 0) * borderWindSpeed * descentRate
               - Vector3D(u * sin(rad), u * cos(rad), v);
    } else {  // Free atomosphere
        return Vector3D(Atmospehre::Wind::GeostrophicWind, 0, 0);
    }
}

Vector3D WindModel::getWindOnlyPowerLow() {
    if (m_height < 0) {
        return Vector3D();
    } else {
        const double rad    = m_groundWindDirection * Constant::PI / 180;
        const Vector3D wind = -Vector3D(sin(rad), cos(rad), 0) * m_groundWindSpeed;

        return applyPowerLow(wind, m_height);
    }
}
