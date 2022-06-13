#pragma once

#include <string>

enum class WindModelType { Real, Original, OnlyPowerLow, NoWind };

namespace AppSetting {
    namespace Processing {
        extern const bool multiThread;
        extern const size_t threadCount;
    }

    namespace Simulation {
        extern const double dt;
        extern const double detectPeakThreshold;

        // scatter
        extern const double windSpeedMin;
        extern const double windSpeedMax;
        extern const double windDirInterval;
    }

    namespace Result {
        extern const int precision;
        extern const int stepSaveInterval;
    }

    namespace WindModel {
        extern const double powerConstant;
        extern const double powerLowBaseAltitude;
        extern const WindModelType type;
        extern const std::string realdataFilename;
    }
}
