#pragma once

#include <string>
#include <vector>

#include "math/Vector3D.hpp"

struct WindData {
    double height    = 0;
    double speed     = 0;
    double direction = 0;

    bool operator==(const WindData& w) {
        return (height == w.height && speed == w.speed && direction == w.direction);
    }
};

class Air {
    std::vector<WindData> windData_;

    const double m_groundWindSpeed, m_groundWindDirection;

    double m_directionInterval = 0.0;

    double m_height             = 0.0;
    double m_geopotentialHeight = 0.0;
    double m_gravity            = 0.0;
    double m_temperature        = 0.0;
    double m_pressure           = 0.0;
    double m_airDensity         = 0.0;
    Vector3D m_wind;

    bool m_initialized = false;

public:
    Air();  // real

    Air(double groundWindSpeed, double groundWindDirection);  // original or only_powerlow

    void update(double height);

    Vector3D wind() const {
        return m_wind;
    }

    double density() const {
        return m_airDensity;
    }

    double gravity() const {
        return m_gravity;
    }

    bool initialized() const {
        return m_initialized;
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
