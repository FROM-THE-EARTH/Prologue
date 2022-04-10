#pragma once

#include "dynamics/WindModel.hpp"
#include "env/Environment.hpp"
#include "env/Map.hpp"
#include "rocket/Rocket.hpp"
#include "rocket/RocketSpec.hpp"

enum class TrajectoryMode : int { Trajectory = 1, Parachute };

enum class RocketType : int { Single = 1, Multi };

enum class DetachType : int { BurningFinished = 1, Time, SyncPara, DoNotDeatch };

struct SimuResult {
    // condition
    const double windSpeed     = 0.0;
    const double windDirection = 0.0;

    // spec
    const RocketSpec rocketSpec;

    // data
    std::vector<Rocket> timeSeriesRockets;

    SimuResult(double _windSpeed, double _windDirection, const RocketSpec& _rocketSpec) :
        windSpeed(_windSpeed), windDirection(_windDirection), rocketSpec(_rocketSpec) {
        timeSeriesRockets.reserve(80000);
    }

    void organize() {
        for (auto& rocket : timeSeriesRockets) {
            for (auto& body : rocket.bodies) {
                // fix z
                if (body.pos.z < 0) {
                    body.pos.z = 0.0;
                }
            }
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
    double m_maxHeight      = 0;
    double m_detectPeakTime = 0;

    // result
    std::shared_ptr<SimuResult> m_result = nullptr;

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
        m_rocket.bodies.resize(m_rocketSpec.rocketParam.size());
    }

    bool run(double windSpeed, double windDirection);

    std::shared_ptr<SimuResult> getResult() const {
        return m_result;
    }

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
