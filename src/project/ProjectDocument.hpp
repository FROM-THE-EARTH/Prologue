// ------------------------------------------------
// GUI/CLIで共有する、保存可能なプロジェクト諸元
// ------------------------------------------------

#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace project {
    inline constexpr int CurrentFormatVersion = 2;

    struct Environment {
        std::string place;
        double latitude = 0.0;
        double longitude = 0.0;
        int coordinateZone = 0;
        double magneticDeclination = 0.0;
        double railLength = 0.0;
        double railAzimuth = 0.0;
        double railElevation = 0.0;
    };

    struct Parachute {
        std::optional<double> openingTimeFromLaunch;
        std::optional<double> openingTimeFromPeak;
        std::optional<double> openingHeight;
        double CdS = 0.0;
    };

    struct Transition {
        double time = 0.0;
        double mass = 0.0;
        double Cd = 0.0;
    };

    struct Engine {
        std::optional<std::filesystem::path> thrustFile;
        double thrustMeasuredPressure = 101325.0;
        double nozzleDiameter = 0.0;
    };

    struct Aerodynamics {
        std::optional<std::filesystem::path> coefficientFile;
        double centerOfPressure = 0.0;
        double centerOfPressureAlpha = 0.0;
        double dragCoefficientInitial = 0.0;
        double dragCoefficientFinal = 0.0;
        double dragCoefficientAlphaSquared = 0.0;
        double normalForceCoefficient = 0.0;
    };

    struct Body {
        double length = 0.0;
        double diameter = 0.0;
        double massInitial = 0.0;
        double massFinal = 0.0;
        double centerOfGravityInitial = 0.0;
        double centerOfGravityFinal = 0.0;
        double pitchYawMomentOfInertiaInitial = 0.0;
        double pitchYawMomentOfInertiaFinal = 0.0;
        double pitchDampingMomentCoefficient = 0.0;
        std::vector<Parachute> parachutes;
        Engine engine;
        Aerodynamics aerodynamics;
        std::vector<Transition> transitions;
    };

    struct Separation {
        size_t sourceBodyIndex = 0;
        std::vector<size_t> productBodyIndices;
    };

    struct Document {
        int formatVersion = CurrentFormatVersion;
        Environment environment;
        std::vector<Body> bodies;
        std::vector<Separation> separations;
    };
}
