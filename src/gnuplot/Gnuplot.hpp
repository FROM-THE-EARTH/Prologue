#pragma once

#include <string>
#include <vector>

struct MapData;
struct ResultRocket;

namespace Gnuplot {
    void Initialize(const char* dirname, MapData mapData);

    void Plot(const ResultRocket& result);

    void Plot(const std::vector<ResultRocket>& result);

    void Show();

    void Save();
}
