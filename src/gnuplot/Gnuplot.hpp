#pragma once

#include <string>
#include <vector>

struct SolvedResult;

enum class MapType;

namespace Gnuplot {
    void Initialize(const char* dirname, MapType mapType);

    void Plot(const SolvedResult& result);

    void Plot(const std::vector<SolvedResult>& result);

    void Show();

    void Save();
}
