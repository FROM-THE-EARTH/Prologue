#pragma once

#include "dynamics/Air.hpp"
#include "env/Map.hpp"
#include "rocket/Rocket.hpp"
#include "rocket/RocketSpec.hpp"

enum class TrajectoryMode : int { Trajectory = 1, Parachute };

enum class RocketType : int { Single = 1, Multi };

enum class DetachType : int { BurningFinished = 1, Time, SyncPara, DoNotDeatch };

struct ResultOfEachRocket {
    // all flight data
    std::vector<Rocket> flightData;

    // max, min values
    double maxHeight      = 0.0;
    double maxVelocity    = 0.0;
    double maxAttackAngle = 0.0;
    double maxNormalForce = 0.0;

    // special values
    double detectPeakTime     = 0.0;
    double timeAtParaOpened   = 0.0;
    double heightAtParaOpened = 0.0;
    double airVelAtParaOpened = 0.0;
    double terminalVelocity   = 0.0;
    double terminalTime       = 0.0;

    // pos
    double lenFromLaunchPoint = 0.0;
    double latitude           = 0.0;
    double longitude          = 0.0;

    ResultOfEachRocket() {
        flightData.reserve(80000);
    }
};

struct SolvedResult {
    std::vector<ResultOfEachRocket> rocket = std::vector<ResultOfEachRocket>(1);

    // special values
    double windSpeed           = 0.0;
    double windDirection       = 0.0;
    double launchClearVelocity = 0.0;

    void organize(MapData map) {
        for (auto& r : rocket) {
            if (r.flightData[r.flightData.size() - 1].pos.z < 0) {
                r.flightData[r.flightData.size() - 1].pos.z = 0.0;  // landing point
            }
            r.lenFromLaunchPoint = r.flightData[r.flightData.size() - 1].pos.length();
            r.latitude           = map.coordinate.latitudeAt(r.flightData[r.flightData.size() - 1].pos.y);
            r.longitude          = map.coordinate.longitudeAt(r.flightData[r.flightData.size() - 1].pos.x);
        }
    }
};

class Solver {
    // config
    const double m_dt;
    const RocketType m_rocketType;
    const TrajectoryMode m_trajectoryMode;
    const DetachType m_detachType;
    const double m_detachTime;

    // spec
    RocketSpec m_rocketSpec;

    // rocket
    Rocket m_rocket, rocketDelta_;
    std::vector<Rocket> m_rocketAtDetached;

    // dynamics
    Air* m_air = nullptr;
    Vector3D m_force_b;
    Vector3D m_moment_b;

    // map
    MapData m_mapData;

    // stastus
    bool m_launchClear         = false;
    double m_combustionTime    = 0.0;
    size_t m_targetRocketIndex = 0;

    // result
    SolvedResult m_result;

public:
    Solver(double dt,
           MapData mapData,
           RocketType rocketType,
           TrajectoryMode mode,
           DetachType detachType,
           double detachTime,
           const RocketSpec& spec) :
        m_dt(dt),
        m_mapData(mapData),
        m_rocketType(rocketType),
        m_trajectoryMode(mode),
        m_detachType(detachType),
        m_detachTime(detachTime),
        m_rocketSpec(spec) {}

    ~Solver() {
        delete m_air;
    }

    bool run(double windSpeed, double windDirection);

    SolvedResult getResult() const {
        return m_result;
    }

private:
    void initializeParameters();

    void update();

    void updateParachuteStatus();

    void updateDetachedStatus();

    void updateParameters();

    void calcDynamicForce();

    void updateDelta();

    void finalUpdate();
};
