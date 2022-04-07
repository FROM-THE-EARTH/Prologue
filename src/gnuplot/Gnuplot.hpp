#pragma once

#include <memory>
#include <string>
#include <vector>

struct MapData;
struct SimuResult;

namespace Gnuplot {
    void Initialize(const char* dirname, MapData mapData);

    void Plot(const std::shared_ptr<SimuResult>& result);

    void Plot(const std::vector<std::shared_ptr<SimuResult>>& result);

    void Show();

    void Save();
}
