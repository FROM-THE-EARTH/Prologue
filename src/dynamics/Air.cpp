#include "Air.hpp"

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

Air::Air(double groundWindSpeed, double groundWindDirection) :
    m_groundWindSpeed(groundWindSpeed), m_groundWindDirection(groundWindDirection) {
    m_directionInterval = 270 - m_groundWindDirection;
    if (m_directionInterval <= -45.0) {
        m_directionInterval = 270 - m_groundWindDirection + 360;
    }

    m_initialized = true;
}

Air::Air() : m_groundWindSpeed(0.0), m_groundWindDirection(0.0) {
    if (!windData_.empty()) {
        m_initialized = true;
        return;
    }

    std::fstream windfile("input/wind/" + AppSetting::GetSetting().windModel.realdataFilename);

    char header[1024];
    windfile.getline(header, 1024);
    size_t i = 1;
    char c;
    std::string dummy;
    windData_.push_back(WindData());
    while (!windfile.eof()) {
        windData_.push_back(WindData());
        windfile >> windData_[i].height >> c >> windData_[i].speed >> c >> windData_[i].direction;
        if (windData_[i] == WindData()) {
            break;
        }
        i++;
    }
    if (windData_[windData_.size() - 1] == WindData()) {
        windData_.pop_back();
    }

    windfile.close();

    if (windData_.size() == 0) {
        m_initialized = false;
        return;
    }

    m_initialized = true;
}

void Air::update(double height) {
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

double Air::getGeopotentialHeight() {
    return Constant::EarthRadius * m_height / (Constant::EarthRadius + m_height);
}

double Air::getGravity() {
    const double k = (Constant::EarthRadius / (Constant::EarthRadius + m_height));
    return Constant::G * k * k;
}

double Air::getTemperature() {
    for (size_t i = 0; i < N; i++) {
        if (m_geopotentialHeight < heightList[i + 1])
            return baseTemperature[i] + tempDecayRate[i] * (m_geopotentialHeight - heightList[i])
                   + Constant::AbsoluteZero;
    }

    CommandLine::PrintInfo(PrintInfoType::Error, "Not defined above 32000m");

    return 0;
}

double Air::getPressure() {
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

double Air::getAirDensity() {
    return m_pressure / ((m_temperature - Constant::AbsoluteZero) * Constant::GasConstant);
}

Vector3D Air::getWindFromData() {
    size_t index = 0;
    for (size_t i = 0; i < windData_.size(); i++) {
        if (m_height > windData_[i].height) {
            index++;
        }
    }

    if (index == 0) {
        return Vector3D(0, 0, 0);
    }

    const double windSpeed = windData_[index - 1].speed
                             + (windData_[index].speed - windData_[index - 1].speed)
                                   / (windData_[index].height - windData_[index - 1].height)
                                   * (m_height - windData_[index - 1].height);

    const double direction = windData_[index - 1].direction
                             + (windData_[index].direction - windData_[index - 1].direction)
                                   / (windData_[index].height - windData_[index - 1].height)
                                   * (m_height - windData_[index - 1].height);

    const double rad = direction * Constant::PI / 180;

    return -Vector3D(sin(rad), cos(rad), 0) * windSpeed;
}

Vector3D Air::getWindOriginalModel() {
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

Vector3D Air::getWindOnlyPowerLow() {
    if (m_height <= 0) {
        const double rad = m_groundWindDirection * Constant::PI / 180;
        return -Vector3D(sin(rad), cos(rad), 0) * m_groundWindSpeed;
    } else {
        const double rad    = m_groundWindDirection * Constant::PI / 180;
        const Vector3D wind = -Vector3D(sin(rad), cos(rad), 0) * m_groundWindSpeed;

        return wind * pow(m_height * 0.5, 1.0 / AppSetting::GetSetting().windModel.powerConstant);
    }
}
