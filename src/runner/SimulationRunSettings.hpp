// ------------------------------------------------
// シミュレーションの実行方法に関する設定
// ------------------------------------------------

#pragma once

#include <cstddef>

enum class SimulationMode : int { Scatter = 1, Detail };

struct SimulationExecutionSettings {
    bool multiThread = false;
    size_t threadCount = 1;
};

struct ScatterRunSettings {
    double windSpeedMin = 0.0;
    double windSpeedMax = 0.0;
    double windDirectionInterval = 0.0;
};
