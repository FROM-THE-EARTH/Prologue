#pragma once

#include "gui/RunResults.hpp"
#include "gui/map/MapWidget.hpp"

namespace gui {
    std::vector<MapSeries> BuildMapSeries(const RunResults& results);
}
