#pragma once

#include <memory>
#include <string>
#include <vector>

struct MapData;
struct SimuResultSummary;

namespace Gnuplot {
    void Initialize(const char* dirname, MapData mapData);

    void Plot(const SimuResultSummary& result);

    void Plot(const std::vector<SimuResultSummary>& results);

    void Show();

    void Save();
}
