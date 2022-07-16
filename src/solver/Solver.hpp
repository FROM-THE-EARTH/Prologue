#pragma once

#include "dynamics/WindModel.hpp"
#include "env/Environment.hpp"
#include "env/Map.hpp"
#include "result/SimuResult.hpp"
#include "rocket/Rocket.hpp"
#include "rocket/RocketSpec.hpp"

enum class TrajectoryMode : int { Trajectory = 1, Parachute };

enum class RocketType : int { Single = 1, Multi };

enum class DetachType : int { BurningFinished = 1, Time, SyncPara, DoNotDeatch };

class Solver {
    // config
    const double m_dt;
    const RocketType m_rocketType;
    const TrajectoryMode m_trajectoryMode;
    const DetachType m_detachType;
    const double m_detachTime;

    // spec
    const RocketSpec m_rocketSpec;

    // rocket
    Rocket m_rocket;
    Body m_bodyDelta;

    // env
    std::unique_ptr<WindModel> m_windModel;
    Environment m_environment;
    MapData m_mapData;

    // stastus
    size_t m_currentBodyIndex = 0;  // index of the body being solved
    size_t m_detachCount      = 0;

    // calc
    double m_maxAltitude    = 0;
    double m_detectPeakTime = 0;

    // Simulation
    size_t m_steps = 0;

    // result
    std::shared_ptr<SimuResultLogger> m_resultLogger = nullptr;

public:
    Solver(double dt,
           MapData mapData,
           RocketType rocketType,
           TrajectoryMode mode,
           DetachType detachType,
           double detachTime,
           const Environment& env,
           const RocketSpec& spec) :
        m_dt(dt),
        m_rocketType(rocketType),
        m_trajectoryMode(mode),
        m_detachType(detachType),
        m_detachTime(detachTime),
        m_rocketSpec(spec),
        m_environment(env),
        m_mapData(mapData) {
        m_rocket.bodies.resize(m_rocketSpec.bodySpec.size());
    }

    std::shared_ptr<SimuResultLogger> solve(double windSpeed, double windDirection);

private:
    void initializeRocket();

    void update();

    void updateParachute();

    bool updateDetachment();

    void updateAerodynamicParameters();

    void updateRocketProperties();

    void updateExternalForce();

    void updateRocketDelta();

    void applyDelta();

    void organizeResult();

    // prepare the next rocket (multi rocket)
    void nextRocket();
};
