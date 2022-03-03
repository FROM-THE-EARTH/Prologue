#include "Map.hpp"

namespace Map {
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
