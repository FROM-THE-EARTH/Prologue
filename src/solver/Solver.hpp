#pragma once

#include "dynamics/WindModel.hpp"
#include "env/Environment.hpp"
#include "env/Map.hpp"
#include "rocket/Rocket.hpp"
#include "rocket/RocketSpec.hpp"

enum class TrajectoryMode : int { Trajectory = 1, Parachute };

enum class RocketType : int { Single = 1, Multi };

enum class DetachType : int { BurningFinished = 1, Time, SyncPara, DoNotDeatch };

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
};

class SimuResultLogger {
private:
    MapData m_map;
    SimuResultSummary m_result;

public:
    SimuResultLogger(const MapData& map, double windSpeed, double windDirection) : m_map(map) {
        m_result.windSpeed     = windSpeed;
        m_result.windDirection = windDirection;
    }

    SimuResultSummary getResult() const {
        return m_result;
    }

    SimuResultSummary getResultScatterFormat() const {
        SimuResultSummary result = m_result;

        // remove steps that are not landing points
        for (auto& body : result.bodyResults) {
            const SimuResultStep lastBody = body.steps[body.steps.size() - 1];
            body.steps                    = std::vector<SimuResultStep>(1, lastBody);
        }

        // remove body result that not contain velid landing point
        for (int i = static_cast<int>(result.bodyResults.size() - 1); i >= 0; i--) {
            if (result.bodyResults[i].steps[0].rocket_pos.z != 0.0) {
                result.bodyResults.erase(result.bodyResults.begin() + i);
            }
        }

        return result;
    }

    void pushBody() {
        m_result.bodyResults.emplace_back();
    }

    void setLaunchClear(const Body& body) {
        m_result.launchClearTime     = body.elapsedTime;
        m_result.launchClearVelocity = body.velocity;
    }

    void update(size_t bodyIndex, const Rocket& rocket, const Body& body, const WindModel& windModel) {
        SimuResultStep step;

        // General
        step.gen_timeFromLaunch = rocket.timeFromLaunch;
        step.gen_elapsedTime    = body.elapsedTime;

        // Air
        step.air_dencity = windModel.density();
        step.air_gravity = windModel.gravity();
        step.air_wind    = windModel.wind();

        // Body
        step.rocket_mass        = body.mass;
        step.rocket_cgLength    = body.reflLength;
        step.rocket_iyz         = body.iyz;
        step.rocket_ix          = body.ix;
        step.rocket_attackAngle = body.attackAngle;
        step.rocket_pos         = body.pos;
        step.rocket_velocity    = body.velocity;
        step.rocket_airspeed_b  = body.airSpeed_b;
        step.rocket_force_b     = body.force_b;
        step.Cnp                = body.Cnp;
        step.Cny                = body.Cny;
        step.Cmqp               = body.Cmqp;
        step.Cmqy               = body.Cmqy;
        step.Cp                 = body.aeroCoef.Cp;
        step.Cd                 = body.aeroCoef.Cd;
        step.Cna                = body.aeroCoef.Cna;

        step.latitude           = m_map.coordinate.latitudeAt(body.pos.y);
        step.longitude          = m_map.coordinate.longitudeAt(body.pos.x);
        step.lenFromLaunchPoint = body.pos.length();

        m_result.bodyResults[bodyIndex].steps.emplace_back(step);
    }

    void organize() {
        for (auto& bodyResult : m_result.bodyResults) {
            for (auto& step : bodyResult.steps) {
                if (step.rocket_pos.z < 0) {
                    step.rocket_pos.z = 0.0;
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
        m_rocket.bodies.resize(m_rocketSpec.rocketParam.size());
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
