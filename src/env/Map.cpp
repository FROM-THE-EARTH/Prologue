// ------------------------------------------------
// Map.hppの実装
// ------------------------------------------------

#include "Map.hpp"

#include <memory>

#include "utils/JsonUtils.hpp"

namespace Map {
    namespace Internal {
        std::unique_ptr<boost::property_tree::ptree> pt;

        template <typename T>
        T GetValue(const std::string& key) {
            if (!pt) {
                pt = std::make_unique<boost::property_tree::ptree>();
                boost::property_tree::read_json("input/map/config.json", *pt);
            }
            return JsonUtils::GetValueExc<T>(*pt, key);
        }
    }

    const MapData NoshiroLand("nosiro_land",
                              MapType::NOSIRO_LAND,
                              Internal::GetValue<double>("nosiro_land.magnetic_declination"),
                              Internal::GetValue<double>("nosiro_land.latitude"),
                              Internal::GetValue<double>("nosiro_land.longitude"),
                              "nosiro_land.png",
                              1.0,
                              1.0,
                              -730,
                              -1140);

    const MapData NoshiroSea("nosiro_sea",
                             MapType::NOSIRO_SEA,
                             Internal::GetValue<double>("nosiro_sea.magnetic_declination"),
                             Internal::GetValue<double>("nosiro_sea.latitude"),
                             Internal::GetValue<double>("nosiro_sea.longitude"),
                             "nosiro_sea.png",
                             7.0,
                             7.0,
                             -8700,
                             -3650);

    const MapData IzuLand("izu_land",
                          MapType::IZU_LAND,
                          Internal::GetValue<double>("izu_land.magnetic_declination"),
                          Internal::GetValue<double>("izu_land.latitude"),
                          Internal::GetValue<double>("izu_land.longitude"),
                          "izu_land.png",
                          1.00,
                          1.00,
                          -750,
                          -820);

    const MapData IzuSea("izu_sea",
                         MapType::IZU_SEA,
                         Internal::GetValue<double>("izu_sea.magnetic_declination"),
                         Internal::GetValue<double>("izu_sea.latitude"),
                         Internal::GetValue<double>("izu_sea.longitude"),
                         "izu_sea.png",
                         5.77,
                         5.77,
                         -2380,
                         -5640);

    std::optional<MapData> GetMap(const std::string& key) {
        if (key == "nosiro_land") {
            return NoshiroLand;
        }

        if (key == "nosiro_sea") {
            return NoshiroSea;
        }

        if (key == "izu_land") {
            return IzuLand;
        }

        if (key == "izu_sea") {
            return IzuSea;
        }

        return std::nullopt;
    }

    std::optional<MapData> GetMap(MapType type) {
        switch (type) {
        case MapType::NOSIRO_LAND:
            return NoshiroLand;

        case MapType::NOSIRO_SEA:
            return NoshiroSea;

        case MapType::IZU_LAND:
            return IzuLand;

        case MapType::IZU_SEA:
            return IzuSea;

        default:
            return std::nullopt;
        }
    }
}
