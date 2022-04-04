#include "Solver.hpp"

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "env/Map.hpp"

#define THIS_ROCKET_SPEC m_rocketSpec.rocketParam[m_targetRocketIndex]
#define THIS_ROCKET_RESULT m_result.rocket[m_targetRocketIndex]

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

    // loop until all rockets are solved
    // single rocket: solve once
    // multi rokcet : every rockets including after detachment
    do {
        initializeRocket();

        // loop until the rocket lands
        while (m_rocket.pos.z > 0.0 || m_rocket.elapsedTime < 0.1) {
            update();

            if (m_trajectoryMode == TrajectoryMode::Parachute) {
                updateParachute();
            }

            if (m_rocketType == RocketType::Multi) {
                updateDetachment();
            }

            updateAerodynamicParameters();

            updateRocketProperties();

            updateExternalForce();

            updateRocketDelta();

            applyDelta();

            organizeResult();
        }

        landingCount++;

    } while (landingCount <= m_rocketAtDetached.size());

    return true;
}

void Solver::initializeRocket() {
    // Second, Third Rocket(Multiple)
    if (m_rocketAtDetached.size() != 0 && m_rocketAtDetached.size() + 1 > m_targetRocketIndex) {
        m_rocket = m_rocketAtDetached[m_targetRocketIndex - 1];

        nextRocket();

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
}

void Solver::updateParachute() {
    const bool detectpeakConditon =
        THIS_ROCKET_RESULT.maxHeight > m_rocket.pos.z + AppSetting::Simulation::detectPeakThreshold;

    if (detectpeakConditon && !m_rocket.detectPeak) {
        m_rocket.detectPeak = true;
    }

    if (m_rocket.parachuteOpened) {
        return;
    }

    const bool detectpeak = THIS_ROCKET_SPEC.parachute[0].openingType == ParaOpenType::DetectPeak;

    const bool fixedtime          = THIS_ROCKET_SPEC.parachute[0].openingType == ParaOpenType::FixedTime;
    const bool fixedtimeCondition = m_rocket.elapsedTime > THIS_ROCKET_SPEC.parachute[0].openingTime;

    const bool time_from_detect_peak = THIS_ROCKET_SPEC.parachute[0].openingType == ParaOpenType::TimeFromDetectPeak;

    if ((detectpeak && detectpeakConditon) || (fixedtime && fixedtimeCondition)) {
        m_rocket.parachuteOpened              = true;
        THIS_ROCKET_RESULT.timeAtParaOpened   = m_rocket.elapsedTime;
        THIS_ROCKET_RESULT.airVelAtParaOpened = m_rocket.airSpeed_b.length();
        THIS_ROCKET_RESULT.heightAtParaOpened = m_rocket.pos.z;
    }

    const bool time_from_detect_peakCondition =
        m_rocket.elapsedTime - THIS_ROCKET_RESULT.detectPeakTime > THIS_ROCKET_SPEC.parachute[0].openingTime;

    if (time_from_detect_peak) {
        if (!m_rocket.waitForOpenPara && detectpeakConditon) {
            m_rocket.waitForOpenPara = true;
        }
        if (m_rocket.waitForOpenPara && time_from_detect_peakCondition) {
            m_rocket.parachuteOpened              = true;
            THIS_ROCKET_RESULT.timeAtParaOpened   = m_rocket.elapsedTime;
            THIS_ROCKET_RESULT.airVelAtParaOpened = m_rocket.airSpeed_b.length();
            THIS_ROCKET_RESULT.heightAtParaOpened = m_rocket.pos.z;
            m_rocket.waitForOpenPara              = false;
        }
    }
}

void Solver::updateDetachment() {
    bool detachCondition = false;

    switch (m_detachType) {
    case DetachType::BurningFinished:
        detachCondition = THIS_ROCKET_SPEC.engine.isFinishBurning(m_combustionTime);
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
            sumThrust += THIS_ROCKET_SPEC.engine.thrustAt(t) * (0.2 - t) / 0.2;
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
        nextRocket();
    }
}

void Solver::updateAerodynamicParameters() {
    if ((m_rocket.velocity - m_windModel->wind()).length() != 0) {
        m_rocket.airSpeed_b = (m_rocket.velocity - m_windModel->wind()).applyQuaternion(m_rocket.quat.conjugated());
    } else {
        m_rocket.airSpeed_b = Vector3D();
    }

    m_rocket.attackAngle =
        atan(sqrt(m_rocket.airSpeed_b.y * m_rocket.airSpeed_b.y + m_rocket.airSpeed_b.z * m_rocket.airSpeed_b.z)
             / (m_rocket.airSpeed_b.x + 1e-16));

    m_rocket.aeroCoef = THIS_ROCKET_SPEC.aeroCoefStorage.valuesIn(m_rocket.airSpeed_b.length(), m_rocket.attackAngle);

    const double alpha = atan(m_rocket.airSpeed_b.z / (m_rocket.airSpeed_b.x + 1e-16));
    const double beta  = atan(m_rocket.airSpeed_b.y / (m_rocket.airSpeed_b.x + 1e-16));

    m_rocket.Cnp = m_rocket.aeroCoef.Cna * alpha;
    m_rocket.Cny = m_rocket.aeroCoef.Cna * beta;

    m_rocket.Cmqp = THIS_ROCKET_SPEC.Cmq;
    m_rocket.Cmqy = THIS_ROCKET_SPEC.Cmq;
}

void Solver::updateRocketProperties() {
    if (m_combustionTime <= THIS_ROCKET_SPEC.engine.combustionTime()) {
        m_rocketDelta.mass =
            (THIS_ROCKET_SPEC.massFinal - THIS_ROCKET_SPEC.massInitial) / THIS_ROCKET_SPEC.engine.combustionTime();
        m_rocketDelta.reflLength = (THIS_ROCKET_SPEC.CGLengthFinal - THIS_ROCKET_SPEC.CGLengthInitial)
                                   / THIS_ROCKET_SPEC.engine.combustionTime();
        m_rocketDelta.iyz = (THIS_ROCKET_SPEC.rollingMomentInertiaFinal - THIS_ROCKET_SPEC.rollingMomentInertiaInitial)
                            / THIS_ROCKET_SPEC.engine.combustionTime();
        m_rocketDelta.ix = (0.02 - 0.01) / 3;
    } else {
        m_rocketDelta.mass       = 0;
        m_rocketDelta.reflLength = 0;
        m_rocketDelta.iyz        = 0;
        m_rocketDelta.ix         = 0;
    }
}

void Solver::updateExternalForce() {
    m_force_b  = Vector3D(0, 0, 0);
    m_moment_b = Vector3D(0, 0, 0);

    // Thrust
    m_force_b.x += THIS_ROCKET_SPEC.engine.thrustAt(m_combustionTime);

    if (!m_rocket.parachuteOpened) {
        // Aero
        const double preForceCalc = 0.5 * m_windModel->density() * m_rocket.airSpeed_b.length()
                                    * m_rocket.airSpeed_b.length() * THIS_ROCKET_SPEC.bottomArea;
        m_force_b.x -= m_rocket.aeroCoef.Cd * preForceCalc * cos(m_rocket.attackAngle);
        m_force_b.y -= m_rocket.Cny * preForceCalc;
        m_force_b.z -= m_rocket.Cnp * preForceCalc;

        // Moment
        const double preMomentCalc = 0.25 * m_windModel->density() * m_rocket.airSpeed_b.length()
                                     * THIS_ROCKET_SPEC.length * THIS_ROCKET_SPEC.length * THIS_ROCKET_SPEC.bottomArea;
        m_moment_b.x = 0;
        m_moment_b.y = preMomentCalc * m_rocket.Cmqp * m_rocket.omega_b.y;
        m_moment_b.z = preMomentCalc * m_rocket.Cmqy * m_rocket.omega_b.z;

        m_moment_b.y += m_force_b.z * (m_rocket.aeroCoef.Cp - m_rocket.reflLength);
        m_moment_b.z -= m_force_b.y * (m_rocket.aeroCoef.Cp - m_rocket.reflLength);

        // Gravity
        m_force_b +=
            Vector3D(0, 0, -m_windModel->gravity()).applyQuaternion(m_rocket.quat.conjugated()) * m_rocket.mass;
    }
}

void Solver::updateRocketDelta() {
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
                            * THIS_ROCKET_SPEC.parachute[m_rocket.parachuteIndex].Cd;

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

void Solver::applyDelta() {
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

void Solver::organizeResult() {
    THIS_ROCKET_RESULT.flightData.push_back(m_rocket);

    const bool rising = m_rocket.velocity.z > 0;

    // height
    if (THIS_ROCKET_RESULT.maxHeight < m_rocket.pos.z) {
        THIS_ROCKET_RESULT.maxHeight      = m_rocket.pos.z;
        THIS_ROCKET_RESULT.detectPeakTime = m_rocket.elapsedTime;
    }

    // velocity
    if (THIS_ROCKET_RESULT.maxVelocity < m_rocket.velocity.length()) {
        THIS_ROCKET_RESULT.maxVelocity = m_rocket.velocity.length();
    }

    // terminal velocity
    THIS_ROCKET_RESULT.terminalVelocity = m_rocket.velocity.length();

    // terminal time
    THIS_ROCKET_RESULT.terminalTime = m_rocket.elapsedTime;

    // attack angle
    const double atkang = m_launchClear && rising ? m_rocket.attackAngle * 180 / Constant::PI : 0.0;
    if (THIS_ROCKET_RESULT.maxAttackAngle < atkang) {
        THIS_ROCKET_RESULT.maxAttackAngle = atkang;
    }

    // normal force
    const double nForce = rising ? sqrt(m_force_b.z * m_force_b.z + m_force_b.y * m_force_b.y) : 0.0;
    if (THIS_ROCKET_RESULT.maxNormalForce < nForce) {
        THIS_ROCKET_RESULT.maxNormalForce = nForce;
    }
}

void Solver::nextRocket() {
    m_targetRocketIndex++;

    m_result.rocket.push_back({});
}
