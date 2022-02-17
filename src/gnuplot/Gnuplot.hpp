#pragma once

#include <string>
#include <vector>

struct SolvedResult;

enum class Map;

namespace Gnuplot {
    void Initialize(const char* _dirname, Map m);

    void Plot(const SolvedResult& result);

    void Plot(const std::vector<SolvedResult>& result);

    void Show();

    void Save();
}
