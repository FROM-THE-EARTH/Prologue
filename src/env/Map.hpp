#pragma once

#include "env/MapCoordinate.hpp"
#include "env/MapType.hpp"

namespace Map {
    const MapCoordinate NoshiroSea{MapType::NOSIRO_SEA, 40.242865, 140.010450};

    const MapCoordinate NoshiroLand{MapType::NOSIRO_LAND, 0, 0};

    const MapCoordinate IzuSea{MapType::IZU_SEA, 34.680197, 139.439090};

    const MapCoordinate IzuLand{MapType::IZU_LAND, 34.735972, 139.420944};

    const MapCoordinate Unknown{MapType::UNKNOWN, 0, 0};

    MapCoordinate GetMapFromName(const std::string& name);
}
