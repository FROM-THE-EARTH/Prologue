#pragma once

#include <cmath>
#include <string>

#include "misc/Constant.hpp"

enum class MapType { NOSIRO_SEA, NOSIRO_LAND, IZU_SEA, IZU_LAND, UNKNOWN };

class MapCoordinate {
private:
    double m_degPerLen_latitude;
    double m_degPerLen_longitude;

public:
    const MapType mapType;
    const double latitude;   // launchpoint [deg N]
    const double longitude;  // launchpoint [deg E]

    MapCoordinate(MapType _mapType, double _latitude, double _longitude) :
        mapType(_mapType), latitude(_latitude), longitude(_longitude) {
        m_degPerLen_latitude  = 31.0 / 0.00027778;
        m_degPerLen_longitude = 6378150.0 * std::cos(latitude / 180.0 * Constant::PI) * 2.0 * Constant::PI / 360.0;
    }

    double latitudeAt(double length) const {  // length: from here
        return latitude + length / m_degPerLen_latitude;
    }

    double longitudeAt(double length) const {  // length: from here
        return longitude + length / m_degPerLen_longitude;
    }
};

namespace MapFeature {
    const MapCoordinate NoshiroSea{MapType::NOSIRO_SEA, 40.242865, 140.010450};

    const MapCoordinate NoshiroLand{MapType::NOSIRO_LAND, 0, 0};

    const MapCoordinate IzuSea{MapType::IZU_SEA, 34.680197, 139.439090};

    const MapCoordinate IzuLand{MapType::IZU_LAND, 34.735972, 139.420944};

    const MapCoordinate Unknown{MapType::UNKNOWN, 0, 0};

    MapCoordinate GetMapFromName(const std::string& name);
}
