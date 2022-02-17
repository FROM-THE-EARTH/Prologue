#pragma once

#include <string>

enum class WindModelType { Real, Original, OnlyPowerLow };

namespace Settings {
    struct Processing {
        bool multiThread = false;
    };

    struct Simulation {
        double dt                  = 0.001;
        double detectPeakThreshold = 15.0;

        // scatter
        double windSpeedMin    = 1.0;
        double windSpeedMax    = 7.0;
        double windDirInterval = 30.0;
    };

    struct WindModel {
        double powerConstant = 7.0;
        WindModelType type;
        std::string realdataFilename = "wind_data_template.csv";
    };
}

struct Setting {
    Settings::Processing processing;
    Settings::Simulation simulation;
    Settings::WindModel windModel;
};

class AppSetting {
    static Setting setting_;

public:
    static bool Initialize();

    const static Setting& GetSetting() {
        return setting_;
    }
};
