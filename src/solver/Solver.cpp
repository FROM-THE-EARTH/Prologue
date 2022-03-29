#include "Solver.hpp"

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "env/Map.hpp"

bool Solver::run(double windSpeed, double windDirection) {
    switch (AppSetting::WindModel::type) {
    case WindModelType::Real:
        m_windModel = new WindModel(m_mapData.magneticDeclination);
        break;

    default:
        m_windModel = new WindModel(windSpeed, windDirection, m_mapData.magneticDeclination);
        break;
    }

    if (!m_windModel->initialized()) {
        CommandLine::PrintInfo(PrintInfoType::Error, "Cannot create wind model");
        return false;
    }

    m_result.windSpeed     = windSpeed;
    m_result.windDirection = windDirection;

    size_t landingCount = 0;
    do {
        initializeParameters();

        while (m_rocket.pos.z > 0.0 || m_rocket.elapsedTime < 0.1) {
            update();

            if (m_trajectoryMode == TrajectoryMode::Parachute) {
                updateParachuteStatus();
            }

            if (m_rocketType == RocketType::Multi) {
                updateDetachedStatus();
            }

            updateParameters();

            calcDynamicForce();

            updateDelta();

            finalUpdate();
        }

        landingCount++;

    } while (landingCount <= m_rocketAtDetached.size());

    return true;
}

void Solver::initializeParameters() {
    // Second, Third Rocket(Multiple)
    if (m_rocketAtDetached.size() != 0 && m_rocketAtDetached.size() + 1 > m_targetRocketIndex) {
        m_rocket = m_rocketAtDetached[m_targetRocketIndex - 1];

        m_targetRocketIndex++;

        m_result.rocket.push_back({});

        return;
    }

    // First running
    m_rocketDelta.mass       = m_rocketSpec.rocketParam[0].massInitial;
    m_rocketDelta.reflLength = m_rocketSpec.rocketParam[0].CGLengthInitial;
    m_rocketDelta.iyz        = m_rocketSpec.rocketParam[0].rollingMomentInertiaInitial;
    m_rocketDelta.ix         = 0.02;
    m_rocketDelta.pos        = Vector3D(0, 0, 0);
    m_rocketDelta.velocity   = Vector3D(0, 0, 0);
    m_rocketDelta.omega_b    = Vector3D(0, 0, 0);
    m_rocketDelta.quat =
        Quaternion(m_environment.railElevation, (-m_environment.railAzimuth + 90) - m_mapData.magneticDeclination);

    m_rocket = m_rocketDelta;
}

void Solver::update() {
    m_windModel->update(m_rocket.pos.z);

    const double fixedtime =
        m_rocketAtDetached.size() == 0 ? 0.0 : m_rocketAtDetached[m_rocketAtDetached.size() - 1].elapsedTime;
    m_combustionTime = m_rocket.elapsedTime - fixedtime;

    // next target rocket
    if (m_targetRocketIndex == m_result.rocket.size()) {
        m_result.rocket.push_back({});
    }
}

void Solver::updateParachuteStatus() {
    const bool detectpeakConditon =
        m_result.rocket[m_targetRocketIndex].maxHeight > m_rocket.pos.z + AppSetting::Simulation::detectPeakThreshold;

    if (detectpeakConditon && !m_rocket.detectPeak) {
        m_rocket.detectPeak = true;
    }

    if (m_rocket.parachuteOpened) {
        return;
    }

    const bool detectpeak = m_rocketSpec.rocketParam[m_targetRocketIndex].parachute[0].openingType == 0;

    const bool fixedtime = m_rocketSpec.rocketParam[m_targetRocketIndex].parachute[0].openingType == 1;
    const bool fixedtimeCondition =
        m_rocket.elapsedTime > m_rocketSpec.rocketParam[m_targetRocketIndex].parachute[0].openingTime;

    const bool time_from_detect_peak = m_rocketSpec.rocketParam[m_targetRocketIndex].parachute[0].openingType == 2;

    if ((detectpeak && detectpeakConditon) || (fixedtime && fixedtimeCondition)) {
        m_rocket.parachuteOpened                                = true;
        m_result.rocket[m_targetRocketIndex].timeAtParaOpened   = m_rocket.elapsedTime;
        m_result.rocket[m_targetRocketIndex].airVelAtParaOpened = m_rocket.airSpeed_b.length();
        m_result.rocket[m_targetRocketIndex].heightAtParaOpened = m_rocket.pos.z;
    }

    const bool time_from_detect_peakCondition =
        m_rocket.elapsedTime - m_result.rocket[m_targetRocketIndex].detectPeakTime
        > m_rocketSpec.rocketParam[m_targetRocketIndex].parachute[0].openingTime;

    if (time_from_detect_peak) {
        if (!m_rocket.waitForOpenPara && detectpeakConditon) {
            m_rocket.waitForOpenPara = true;
        }
        if (m_rocket.waitForOpenPara && time_from_detect_peakCondition) {
            m_rocket.parachuteOpened                                = true;
            m_result.rocket[m_targetRocketIndex].timeAtParaOpened   = m_rocket.elapsedTime;
            m_result.rocket[m_targetRocketIndex].airVelAtParaOpened = m_rocket.airSpeed_b.length();
            m_result.rocket[m_targetRocketIndex].heightAtParaOpened = m_rocket.pos.z;
            m_rocket.waitForOpenPara                                = false;
        }
    }
}

void Solver::updateDetachedStatus() {
    bool detachCondition = false;

    switch (m_detachType) {
    case DetachType::BurningFinished:
        detachCondition = m_rocketSpec.rocketParam[m_targetRocketIndex].engine.isFinishBurning(m_combustionTime);
        break;
    case DetachType::Time:
        detachCondition = m_rocket.elapsedTime >= m_detachTime;
        break;
    case DetachType::SyncPara:
        detachCondition = m_rocket.parachuteOpened == true;
        break;
    case DetachType::DoNotDeatch:
        return;
        break;
    }

    if (detachCondition && m_rocketAtDetached.size() < 1) {  // if need rocket4, 5, 6... , this code should be changed
        // thrust poewr
        double sumThrust = 0;
        for (double t = 0; t <= 0.2; t += m_dt) {
            sumThrust += m_rocketSpec.rocketParam[m_targetRocketIndex].engine.thrustAt(t) * (0.2 - t) / 0.2;
        }

        // next rocket status
        Rocket detach;
        detach.mass       = m_rocketSpec.rocketParam[m_targetRocketIndex + 2].massInitial;
        detach.reflLength = m_rocketSpec.rocketParam[m_targetRocketIndex + 2].CGLengthInitial;
        detach.iyz        = m_rocketSpec.rocketParam[m_targetRocketIndex + 2].rollingMomentInertiaInitial;
        detach.ix         = 0.02;
        detach.pos        = m_rocket.pos;
        detach.omega_b    = Vector3D();
        detach.quat       = m_rocket.quat;
        detach.velocity   = m_rocket.velocity;
        m_rocketAtDetached.push_back(detach);

        // update this rocket
        m_rocket.mass       = m_rocketSpec.rocketParam[m_targetRocketIndex + 1].massInitial;
        m_rocket.reflLength = m_rocketSpec.rocketParam[m_targetRocketIndex + 1].CGLengthInitial;
        m_rocket.iyz        = m_rocketSpec.rocketParam[m_targetRocketIndex + 1].rollingMomentInertiaInitial;
        m_rocket.velocity -= Vector3D((sumThrust / m_rocket.mass) * m_dt, 0, 0).applyQuaternion(m_rocket.quat);

        // next part
        m_targetRocketIndex++;
        m_result.rocket.push_back({});
    }
}

void Solver::updateParameters() {
    if ((m_rocket.velocity - m_windModel->wind()).length() != 0) {
        m_rocket.airSpeed_b = (m_rocket.velocity - m_windModel->wind()).applyQuaternion(m_rocket.quat.conjugate());
    } else {
        m_rocket.airSpeed_b = Vector3D();
    }

    m_rocketSpec.rocketParam[m_targetRocketIndex].airspeedParam.update(m_rocket.airSpeed_b.length());

    const double alpha = atan(m_rocket.airSpeed_b.z / (m_rocket.airSpeed_b.x + 1e-16));
    const double beta  = atan(m_rocket.airSpeed_b.y / (m_rocket.airSpeed_b.x + 1e-16));
    m_rocket.attackAngle =
        atan(sqrt(m_rocket.airSpeed_b.y * m_rocket.airSpeed_b.y + m_rocket.airSpeed_b.z * m_rocket.airSpeed_b.z)
             / (m_rocket.airSpeed_b.x + 1e-16));

    m_rocket.Cnp = m_rocketSpec.rocketParam[m_targetRocketIndex].airspeedParam.getParam().Cna * alpha;
    m_rocket.Cny = m_rocketSpec.rocketParam[m_targetRocketIndex].airspeedParam.getParam().Cna * beta;

    m_rocket.Cmqp = m_rocketSpec.rocketParam[m_targetRocketIndex].Cmq;
    m_rocket.Cmqy = m_rocketSpec.rocketParam[m_targetRocketIndex].Cmq;

    if (m_combustionTime <= m_rocketSpec.rocketParam[m_targetRocketIndex].engine.combustionTime()) {
        m_rocketDelta.mass = (m_rocketSpec.rocketParam[m_targetRocketIndex].massFinal
                              - m_rocketSpec.rocketParam[m_targetRocketIndex].massInitial)
                             / m_rocketSpec.rocketParam[m_targetRocketIndex].engine.combustionTime();
        m_rocketDelta.reflLength = (m_rocketSpec.rocketParam[m_targetRocketIndex].CGLengthFinal
                                    - m_rocketSpec.rocketParam[m_targetRocketIndex].CGLengthInitial)
                                   / m_rocketSpec.rocketParam[m_targetRocketIndex].engine.combustionTime();
        m_rocketDelta.iyz = (m_rocketSpec.rocketParam[m_targetRocketIndex].rollingMomentInertiaFinal
                             - m_rocketSpec.rocketParam[m_targetRocketIndex].rollingMomentInertiaInitial)
                            / m_rocketSpec.rocketParam[m_targetRocketIndex].engine.combustionTime();
        m_rocketDelta.ix = (0.02 - 0.01) / 3;
    } else {
        m_rocketDelta.mass       = 0;
        m_rocketDelta.reflLength = 0;
        m_rocketDelta.iyz        = 0;
        m_rocketDelta.ix         = 0;
    }
}

void Solver::calcDynamicForce() {
    m_force_b  = Vector3D(0, 0, 0);
    m_moment_b = Vector3D(0, 0, 0);

    // Thrust
    m_force_b.x += m_rocketSpec.rocketParam[m_targetRocketIndex].engine.thrustAt(m_combustionTime);

    if (!m_rocket.parachuteOpened) {
        // Aero
        const double preForceCalc = 0.5 * m_windModel->density() * m_rocket.airSpeed_b.length()
                                    * m_rocket.airSpeed_b.length()
                                    * m_rocketSpec.rocketParam[m_targetRocketIndex].bottomArea;
        const double cd = m_rocketSpec.rocketParam[m_targetRocketIndex].airspeedParam.getParam().Cd
                          + m_rocketSpec.rocketParam[m_targetRocketIndex].airspeedParam.getParam().Cd_a2
                                * m_rocket.attackAngle * m_rocket.attackAngle;
        m_force_b.x -= cd * preForceCalc * cos(m_rocket.attackAngle);
        m_force_b.y -= m_rocket.Cny * preForceCalc;
        m_force_b.z -= m_rocket.Cnp * preForceCalc;

        // Moment
        const double preMomentCalc = 0.25 * m_windModel->density() * m_rocket.airSpeed_b.length()
                                     * m_rocketSpec.rocketParam[m_targetRocketIndex].length
                                     * m_rocketSpec.rocketParam[m_targetRocketIndex].length
                                     * m_rocketSpec.rocketParam[m_targetRocketIndex].bottomArea;
        m_moment_b.x = 0;
        m_moment_b.y = preMomentCalc * m_rocket.Cmqp * m_rocket.omega_b.y;
        m_moment_b.z = preMomentCalc * m_rocket.Cmqy * m_rocket.omega_b.z;

        const double cp =
            m_rocketSpec.rocketParam[m_targetRocketIndex].airspeedParam.getParam().Cp
            + m_rocketSpec.rocketParam[m_targetRocketIndex].airspeedParam.getParam().Cp_a * m_rocket.attackAngle;
        m_moment_b.y += m_force_b.z * (cp - m_rocket.reflLength);
        m_moment_b.z -= m_force_b.y * (cp - m_rocket.reflLength);

        // Gravity
        m_force_b += Vector3D(0, 0, -m_windModel->gravity()).applyQuaternion(m_rocket.quat.conjugate()) * m_rocket.mass;
    }

    // update delta
    if (m_rocket.pos.length() <= m_environment.railLength && m_rocket.velocity.z >= 0.0) {  // launch
        if (m_force_b.x < 0) {
            m_rocketDelta.pos      = Vector3D();
            m_rocketDelta.velocity = Vector3D();
            m_rocketDelta.omega_b  = Vector3D();
            m_rocketDelta.quat     = Quaternion();
        } else {
            m_force_b.y       = 0;
            m_force_b.z       = 0;
            m_rocketDelta.pos = m_rocket.velocity;

            m_rocketDelta.velocity = m_force_b.applyQuaternion(m_rocket.quat) / m_rocket.mass;

            m_rocketDelta.omega_b = Vector3D();
            m_rocketDelta.quat    = Quaternion();
        }
    } else if (m_rocket.parachuteOpened) {  // parachute opened
        const Vector3D paraSpeed = m_rocket.velocity;
        const double drag        = 0.5 * m_windModel->density() * paraSpeed.z * paraSpeed.z * 1.0
                            * m_rocketSpec.rocketParam[m_targetRocketIndex].parachute[m_rocket.parachuteIndex].Cd;

        m_rocketDelta.velocity.z = drag / m_rocket.mass - m_windModel->gravity();
        m_rocketDelta.velocity.x = 0;
        m_rocketDelta.velocity.y = 0;

        m_rocket.velocity.x = m_windModel->wind().x;
        m_rocket.velocity.y = m_windModel->wind().y;

        m_rocketDelta.pos = m_rocket.velocity;

        m_rocketDelta.omega_b = Vector3D();
        m_rocketDelta.quat    = Quaternion();
    } else if (m_rocket.pos.z < -10) {  // stop simulation
        m_rocketDelta.velocity = Vector3D();
    } else {  // flight
        if (!m_launchClear) {
            m_result.launchClearVelocity = m_rocket.velocity.length();
            m_launchClear                = true;
        }

        m_rocketDelta.pos      = m_rocket.velocity;
        m_rocketDelta.velocity = m_force_b.applyQuaternion(m_rocket.quat) / m_rocket.mass;

        m_rocketDelta.omega_b.x = m_moment_b.x / m_rocket.ix;
        m_rocketDelta.omega_b.y = m_moment_b.y / m_rocket.iyz;
        m_rocketDelta.omega_b.z = m_moment_b.z / m_rocket.iyz;

        m_rocketDelta.quat = m_rocket.quat.angularVelocityApplied(m_rocketDelta.omega_b);
    }
}

void Solver::updateDelta() {
    // Update rocket
    m_rocket.mass += m_rocketDelta.mass * m_dt;
    m_rocket.reflLength += m_rocketDelta.reflLength * m_dt;
    m_rocket.iyz += m_rocketDelta.iyz * m_dt;
    m_rocket.ix += m_rocketDelta.ix * m_dt;
    m_rocket.pos += m_rocketDelta.pos * m_dt;
    m_rocket.velocity += m_rocketDelta.velocity * m_dt;
    m_rocket.omega_b += m_rocketDelta.omega_b * m_dt;
    m_rocket.quat += m_rocketDelta.quat * m_dt;

    m_rocket.quat = m_rocket.quat.normalized();

    m_rocket.elapsedTime += m_dt;
}

void Solver::finalUpdate() {
    m_result.rocket[m_targetRocketIndex].flightData.push_back(m_rocket);

    const bool rising = m_rocket.velocity.z > 0;

    // height
    if (m_result.rocket[m_targetRocketIndex].maxHeight < m_rocket.pos.z) {
        m_result.rocket[m_targetRocketIndex].maxHeight      = m_rocket.pos.z;
        m_result.rocket[m_targetRocketIndex].detectPeakTime = m_rocket.elapsedTime;
    }

    // velocity
    if (m_result.rocket[m_targetRocketIndex].maxVelocity < m_rocket.velocity.length()) {
        m_result.rocket[m_targetRocketIndex].maxVelocity = m_rocket.velocity.length();
    }

    // terminal velocity
    m_result.rocket[m_targetRocketIndex].terminalVelocity = m_rocket.velocity.length();

    // terminal time
    m_result.rocket[m_targetRocketIndex].terminalTime = m_rocket.elapsedTime;

    // attack angle
    const double atkang = m_launchClear && rising ? m_rocket.attackAngle * 180 / Constant::PI : 0.0;
    if (m_result.rocket[m_targetRocketIndex].maxAttackAngle < atkang) {
        m_result.rocket[m_targetRocketIndex].maxAttackAngle = atkang;
    }

    // normal force
    const double nForce = rising ? sqrt(m_force_b.z * m_force_b.z + m_force_b.y * m_force_b.y) : 0.0;
    if (m_result.rocket[m_targetRocketIndex].maxNormalForce < nForce)
        m_result.rocket[m_targetRocketIndex].maxNormalForce = nForce;
}
