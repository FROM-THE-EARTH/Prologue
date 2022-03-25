#include "WindModel.hpp"

#include <fstream>
#include <iostream>

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "misc/Constant.hpp"

// threshold
constexpr size_t N                 = 3;
constexpr double heightList[N + 1] = {0.0, 11000, 20000, 32000};  // height threshold[m]
constexpr double baseTemperature[N]{
    15 - Constant::AbsoluteZero, -56.5 - Constant::AbsoluteZero, -76.5 - Constant::AbsoluteZero};  //[kelvin]
constexpr double tempDecayRate[N] = {-6.5e-3, 0.0, 1e-3};                        // temperature decay rate
constexpr double basePressure[N]  = {Constant::SeaPressure, 22632.064, 5474.9};  //[Pa]

// wind calculation
constexpr double geostrophicWind   = 15;    //[m/s]
constexpr double surfaceLayerLimit = 300;   //[m] Surface layer -300[m]
constexpr double ekmanLayerLimit   = 1000;  //[m] Ekman layer 300-1000[m]

WindModel::WindModel(double groundWindSpeed, double groundWindDirection, double magneticDeclination) :
    m_groundWindSpeed(groundWindSpeed), m_groundWindDirection(groundWindDirection - magneticDeclination) {
    m_directionInterval = 270 - m_groundWindDirection;
    if (m_directionInterval <= -45.0) {
        m_directionInterval = 270 - m_groundWindDirection + 360;
    }

    m_initialized = true;
}

WindModel::WindModel(double magneticDeclination) : m_groundWindSpeed(0.0), m_groundWindDirection(0.0) {
    std::fstream windfile("input/wind/" + AppSetting::GetSetting().windModel.realdataFilename);

    char header[1024];
    windfile.getline(header, 1024);
    size_t i = 1;
    char c;
    std::string dummy;
    m_windData.push_back(WindData());
    while (!windfile.eof()) {
        m_windData.push_back(WindData());
        windfile >> m_windData[i].height >> c >> m_windData[i].speed >> c >> m_windData[i].direction;
        if (m_windData[i] == WindData()) {
            break;
        }
        i++;
    }
    if (m_windData[m_windData.size() - 1] == WindData()) {
        m_windData.pop_back();
    }

    windfile.close();

    for (auto& wind : m_windData) {
        wind.direction -= magneticDeclination;
    }

    m_initialized = true;
}

void WindModel::update(double height) {
    m_height = height;

    m_geopotentialHeight = getGeopotentialHeight();
    m_gravity            = getGravity();
    m_temperature        = getTemperature();
    m_pressure           = getPressure();
    m_airDensity         = getAirDensity();

    switch (AppSetting::GetSetting().windModel.type) {
    case WindModelType::Real:
        m_wind = getWindFromData();
        break;

    case WindModelType::Original:
        m_wind = getWindOriginalModel();
        break;

    case WindModelType::OnlyPowerLow:
        m_wind = getWindOnlyPowerLow();
        break;
    }
}

double WindModel::getGeopotentialHeight() {
    return Constant::EarthRadius * m_height / (Constant::EarthRadius + m_height);
}

double WindModel::getGravity() {
    const double k = (Constant::EarthRadius / (Constant::EarthRadius + m_height));
    return Constant::G * k * k;
}

double WindModel::getTemperature() {
    for (size_t i = 0; i < N; i++) {
        if (m_geopotentialHeight < heightList[i + 1])
            return baseTemperature[i] + tempDecayRate[i] * (m_geopotentialHeight - heightList[i])
                   + Constant::AbsoluteZero;
    }

    CommandLine::PrintInfo(PrintInfoType::Error, "Not defined above 32000m");

    return 0;
}

double WindModel::getPressure() {
    const double temp = m_temperature - Constant::AbsoluteZero;  //[kelvin]

    for (size_t i = 0; i < N; i++) {
        if (m_geopotentialHeight <= heightList[i + 1]) {
            if (i == 1)
                return basePressure[i]
                       * exp(-(m_geopotentialHeight - heightList[i]) * Constant::G
                             / (Constant::GasConstant * baseTemperature[i]));
            else
                return basePressure[i]
                       * pow(baseTemperature[i] / temp, Constant::G / (Constant::GasConstant * tempDecayRate[i]));
        }
    }

    CommandLine::PrintInfo(PrintInfoType::Error, "Not defined above 32000m");

    return 0;
}

double WindModel::getAirDensity() {
    return m_pressure / ((m_temperature - Constant::AbsoluteZero) * Constant::GasConstant);
}

Vector3D WindModel::getWindFromData() {
    size_t index = 0;
    for (size_t i = 0; i < m_windData.size(); i++) {
        if (m_height > m_windData[i].height) {
            index++;
        }
    }

    if (index == 0) {
        return Vector3D(0, 0, 0);
    }

    const double windSpeed = m_windData[index - 1].speed
                             + (m_windData[index].speed - m_windData[index - 1].speed)
                                   / (m_windData[index].height - m_windData[index - 1].height)
                                   * (m_height - m_windData[index - 1].height);

    const double direction = m_windData[index - 1].direction
                             + (m_windData[index].direction - m_windData[index - 1].direction)
                                   / (m_windData[index].height - m_windData[index - 1].height)
                                   * (m_height - m_windData[index - 1].height);

    const double rad = direction * Constant::PI / 180;

    return -Vector3D(sin(rad), cos(rad), 0) * windSpeed;
}

Vector3D WindModel::getWindOriginalModel() {
    if (m_height <= 0) {
        const double rad          = m_groundWindDirection * Constant::PI / 180;
        const Vector3D groundWind = -Vector3D(sin(rad), cos(rad), 0) * m_groundWindSpeed;

        return groundWind;
    }

    if (m_height < surfaceLayerLimit) {  // Surface layer
        const double deltaDirection = m_height / ekmanLayerLimit * m_directionInterval;
        const double rad            = (m_groundWindDirection + deltaDirection) * Constant::PI / 180;
        const Vector3D wind         = -Vector3D(sin(rad), cos(rad), 0) * m_groundWindSpeed;

        return wind * pow(m_height * 0.5, 1.0 / AppSetting::GetSetting().windModel.powerConstant);
    } else if (m_height < ekmanLayerLimit) {  // Ekman layer
        const double deltaDirection = m_height / ekmanLayerLimit * m_directionInterval;
        const double rad            = (m_groundWindDirection + deltaDirection) * Constant::PI / 180;

        const double borderWindSpeed =
            m_groundWindSpeed * pow(m_height * 0.5, 1.0 / AppSetting::GetSetting().windModel.powerConstant);

        const double k = (m_height - surfaceLayerLimit) / (surfaceLayerLimit * sqrt(2));
        const double u = geostrophicWind * (1 - exp(-k) * cos(k));
        const double v = geostrophicWind * exp(-k) * sin(k);

        const double descentRate = ((geostrophicWind - u) / geostrophicWind);

        return -Vector3D(sin(rad), cos(rad), 0) * borderWindSpeed * descentRate
               - Vector3D(u * sin(rad), u * cos(rad), v);
    } else {  // Free atomosphere
        return Vector3D(geostrophicWind, 0, 0);
    }
}

Vector3D WindModel::getWindOnlyPowerLow() {
    if (m_height <= 0) {
        const double rad = m_groundWindDirection * Constant::PI / 180;
        return -Vector3D(sin(rad), cos(rad), 0) * m_groundWindSpeed;
    } else {
        const double rad    = m_groundWindDirection * Constant::PI / 180;
        const Vector3D wind = -Vector3D(sin(rad), cos(rad), 0) * m_groundWindSpeed;

        return wind * pow(m_height * 0.5, 1.0 / AppSetting::GetSetting().windModel.powerConstant);
    }
}
