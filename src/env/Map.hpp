#pragma once

#include <optional>

#include "env/MapCoordinate.hpp"
#include "env/MapType.hpp"

struct MapData {
    std::string key;
    MapType type;
    MapCoordinate coordinate;
    double magneticDeclination;
    std::string imageFileName;
    double gnuplot_dx, gnuplot_dy;
    double gnuplot_origin_x, gnuplot_origin_y;

    MapData() = default;

    MapData(const std::string& keyForJson,
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
        coordinate(MapCoordinate(launchPointLatitude, launchPointLongitude)),
        magneticDeclination(_magneticDeclination),
        imageFileName(imageFileNameForGnuplot),
        gnuplot_dx(dxForGnuplot),
        gnuplot_dy(dyForGnuplot),
        gnuplot_origin_x(originXForGnuplot),
        gnuplot_origin_y(originYForGnuplot) {}
};

namespace Map {
    const MapData NoshiroLand(
        "nosiro_land", MapType::NOSIRO_LAND, 8.9, 40.138624, 139.984906, "nosiro_land.png", 1.0, 1.0, -730, -1140);

    const MapData NoshiroSea(
        "nosiro_sea", MapType::NOSIRO_SEA, 8.94, 40.242865, 140.010450, "nosiro_sea.png", 7.0, 7.0, -8700, -3650);

    const MapData IzuLand(
        "izu_land", MapType::IZU_LAND, 7.53, 34.735972, 139.420944, "izu_land.png", 1.00, 1.00, -750, -820);

    const MapData IzuSea(
        "izu_sea", MapType::IZU_SEA, 7.53, 34.680197, 139.439090, "izu_sea.png", 5.77, 5.77, -2380, -5640);

    std::optional<MapData> GetMap(const std::string& key);

    std::optional<MapData> GetMap(MapType type);
}
