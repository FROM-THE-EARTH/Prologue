// ------------------------------------------------
// マップクラス及び各マップの定義
// ------------------------------------------------

#pragma once

#include <optional>

#include "env/GeoCoordinate.hpp"

enum class MapType { NOSIRO_SEA, NOSIRO_LAND, IZU_SEA, IZU_LAND, UNKNOWN };

struct MapData {
    std::string key;
    MapType type;
    GeoCoordinate coordinate;
    double magneticDeclination;

    MapData() = default;

    explicit MapData(const std::string& keyForJson,
                     MapType mapType,
                     double _magneticDeclination,
                      double launchPointLatitude,
                      double launchPointLongitude,
					 int zone) :
        key(keyForJson),
        type(mapType),
        coordinate(GeoCoordinate(launchPointLatitude, launchPointLongitude, zone)),
        magneticDeclination(_magneticDeclination) {}
};

namespace Map {
    extern const MapData NoshiroLand, NoshiroSea, IzuLand, IzuSea;

    std::optional<MapData> GetMap(const std::string& key);

    std::optional<MapData> GetMap(MapType type);
}
