#pragma once

#include <vector>

#include "dynamics/WindProfile.hpp"
#include "math/Vector3D.hpp"

struct SimulationStep {
    double gen_timeFromLaunch = 0.0;
    double gen_elapsedTime = 0.0;

    bool launchClear = false;
    bool combusting = false;
    std::vector<bool> parachuteOpened;

    double air_density = 0.0;
    double air_gravity = 0.0;
    double air_pressure = 0.0;
    double air_temperature = 0.0;
    Vector3D air_wind;

    double rocket_mass = 0.0;
    double rocket_cgLength = 0.0;
    double rocket_iyz = 0.0;
    double rocket_ix = 0.0;
    double rocket_attackAngle = 0.0;
    Vector3D rocket_pos;
    Vector3D rocket_velocity;
    Vector3D rocket_airspeed_b;
    Vector3D rocket_force_b;
    double Cnp = 0.0;
    double Cny = 0.0;
    double Cmqp = 0.0;
    double Cmqy = 0.0;
    double Cp = 0.0;
    double Cd = 0.0;
    double Cna = 0.0;

    double downrange = 0.0;
    double Fst = 0.0;
    double dynamicPressure = 0.0;
};

struct BodySimulationResult {
    std::vector<SimulationStep> steps;
};

struct SimulationResult {
    std::vector<BodySimulationResult> bodyResults;
    std::vector<Vector3D> bodyFinalPositions;

    double windSpeed = 0.0;
    WindDirection windDirection;

    double launchClearTime = 0.0;
    Vector3D launchClearVelocity;

    double maxAltitude = 0.0;
    double detectPeakTime = 0.0;
    double airspeedAtPeak = 0.0;
    double maxDynamicPressureDuringRising = 0.0;
    double maxDynamicPressureTime = 0.0;
    double maxDynamicPressureAltitude = 0.0;
    double maxAirspeed = 0.0;
    double maxAirspeedTime = 0.0;
    double maxLongitudinalAccel = 0.0;
    double maxLongitudinalAccelTime = 0.0;

    double firstParachuteOpenTime = 0.0;
    double firstParachuteOpenAltitude = 0.0;
    double firstParachuteOpenAirspeed = 0.0;
};
