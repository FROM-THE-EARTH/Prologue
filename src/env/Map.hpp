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
    std::string imageFileName;
    double gnuplot_dx, gnuplot_dy;
    double gnuplot_origin_x, gnuplot_origin_y;

    MapData() = default;

    explicit MapData(const std::string& keyForJson,
                     MapType mapType,
                     double _magneticDeclination,
                     double launchPointLatitude,
                     double launchPointLongitude,
                     std::string imageFileNameForGnuplot,
                     double dxForGnuplot,
                     double dyForGnuplot,
                     double originXForGnuplot,
                     double originYForGnuplot) :
        key(keyForJson),
        type(mapType),
        coordinate(GeoCoordinate(launchPointLatitude, launchPointLongitude)),
        magneticDeclination(_magneticDeclination),
        imageFileName(imageFileNameForGnuplot),
        gnuplot_dx(dxForGnuplot),
        gnuplot_dy(dyForGnuplot),
        gnuplot_origin_x(originXForGnuplot),
        gnuplot_origin_y(originYForGnuplot) {}
};

namespace Map {
    extern const MapData NoshiroLand, NoshiroSea, IzuLand, IzuSea;

    std::optional<MapData> GetMap(const std::string& key);

    std::optional<MapData> GetMap(MapType type);
}
