#include "SimuResult.hpp"

SimuResultLogger::SimuResultLogger(const MapData& map, double windSpeed, double windDirection) : m_map(map) {
    m_result.windSpeed     = windSpeed;
    m_result.windDirection = windDirection;
}

SimuResultSummary SimuResultLogger::getResult() const {
    return m_result;
}

SimuResultSummary SimuResultLogger::getResultScatterFormat() const {
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

void SimuResultLogger::pushBody() {
    m_result.bodyResults.emplace_back();
}

void SimuResultLogger::setLaunchClear(const Body& body) {
    m_result.launchClearTime     = body.elapsedTime;
    m_result.launchClearVelocity = body.velocity;
}

void SimuResultLogger::update(size_t bodyIndex, const Rocket& rocket, const Body& body, const WindModel& windModel) {
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

void SimuResultLogger::organize() {
    for (auto& bodyResult : m_result.bodyResults) {
        for (auto& step : bodyResult.steps) {
            if (step.rocket_pos.z < 0) {
                step.rocket_pos.z = 0.0;
            }
        }
    }
}
