#pragma once

namespace StandardAtmosphere1976 {
    struct State {
        double geopotentialHeight;  // [m]
        double gravity;             // [m/s^2]
        double temperature;         // [deg C]
        double pressure;            // [Pa]
        double density;             // [kg/m^3]
    };

    constexpr double StandardSeaLevelPressure = 101325.0;     // [Pa]
    constexpr double StandardSeaLevelTemperature = 15.0;      // [deg C]
    constexpr double MaximumGeopotentialHeight = 32000.0;     // [m]

    double toGeopotentialHeight(double geometricHeight);

    State calculate(double geometricHeight,
                    double seaLevelPressure = StandardSeaLevelPressure,
                    double seaLevelTemperature = StandardSeaLevelTemperature);
}
