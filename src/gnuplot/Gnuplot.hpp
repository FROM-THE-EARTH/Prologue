#pragma once

#include <string>
#include <vector>

struct MapData;
struct SolvedResult;

namespace Gnuplot {
    void Initialize(const char* dirname, MapData mapData);

    void Plot(const SolvedResult& result);

    void Plot(const std::vector<SolvedResult>& result);

    void Show();

    void Save();
}
