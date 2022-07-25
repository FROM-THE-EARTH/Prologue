// ------------------------------------------------
// 緯度経度に関するクラス
// ------------------------------------------------

#pragma once

#include <cmath>
#include <string>

#include "misc/Constant.hpp"

class GeoCoordinate {
private:
    double m_latitude;   // launchpoint [deg N]
    double m_longitude;  // launchpoint [deg E]
    double m_degPerLen_latitude;
    double m_degPerLen_longitude;

public:
    GeoCoordinate() = default;

    GeoCoordinate(double latitude, double longitude) : m_latitude(latitude), m_longitude(longitude) {
        m_degPerLen_latitude  = 31.0 / 0.00027778;
        m_degPerLen_longitude = 6378150.0 * std::cos(latitude / 180.0 * Constant::PI) * 2.0 * Constant::PI / 360.0;
    }

    double latitude() const {
        return m_latitude;
    }

    double longitude() const {
        return m_longitude;
    }

    double latitudeAt(double length) const {  // length: from here
        return m_latitude + length / m_degPerLen_latitude;
    }

    double longitudeAt(double length) const {  // length: from here
        return m_longitude + length / m_degPerLen_longitude;
    }
};
