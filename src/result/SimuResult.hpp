#pragma once

#include <vector>

#include "dynamics/WindModel.hpp"
#include "env/Map.hpp"
#include "math/Vector3D.hpp"
#include "rocket/Rocket.hpp"

// result of each steps
struct SimuResultStep {
    // General
    double gen_timeFromLaunch = 0;
    double gen_elapsedTime    = 0;

    // Air
    Vector3D air_wind  = Vector3D();
    double air_dencity = 0;
    double air_gravity = 0;
    // double air_pressure    = 0;
    // double air_temperature = 0;

    // Boolean
    bool launchClear     = false;
    bool combusting      = false;
    bool parachuteOpened = false;

    // Body
    double rocket_mass         = 0;
    double rocket_cgLength     = 0;
    double rocket_iyz          = 0;
    double rocket_ix           = 0;
    double rocket_attackAngle  = 0;
    Vector3D rocket_pos        = Vector3D();
    Vector3D rocket_velocity   = Vector3D();
    Vector3D rocket_airspeed_b = Vector3D();
    Vector3D rocket_force_b    = Vector3D();
    double Cnp = 0, Cny = 0;
    double Cmqp = 0, Cmqy = 0;
    double Cp  = 0;
    double Cd  = 0;
    double Cna = 0;

    double latitude           = 0;
    double longitude          = 0;
    double lenFromLaunchPoint = 0;
};

// result of rocket1, rocket2 or rocket3
struct SimuResultBody {
    std::vector<SimuResultStep> steps;
};

struct SimuResultSummary {
    std::vector<SimuResultBody> bodyResults;

    // condition
    double windSpeed     = 0;
    double windDirection = 0;

    // launch clear
    double launchClearTime       = 0;
    Vector3D launchClearVelocity = Vector3D();

    // max
    double maxHeight = 0, detectPeakTime = 0;
    double maxVelocity                = 0;
    double maxAirspeed                = 0;
    double maxNormalForceDuringRising = 0;
};

class SimuResultLogger {
private:
    MapData m_map;
    SimuResultSummary m_result;

public:
    SimuResultLogger(const MapData& map, double windSpeed, double windDirection);

    SimuResultSummary getResult() const;

    SimuResultSummary getResultScatterFormat() const;

    void pushBody();

    void setLaunchClear(const Body& body);

    void update(size_t bodyIndex, const Rocket& rocket, const Body& body, const WindModel& windModel, bool combusting);

    void organize();
};
