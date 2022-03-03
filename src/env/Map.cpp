#include "Map.hpp"

namespace Map {
    MapCoordinate GetMapFromName(const std::string& name) {
        if (name == "nosiro_sea") {
            return NoshiroSea;
        }

        if (name == "nosiro_land") {
            return NoshiroLand;
        }

        if (name == "izu_sea") {
            return IzuSea;
        }

        if (name == "izu_land") {
            return IzuLand;
        }

        return Unknown;
    }
}
