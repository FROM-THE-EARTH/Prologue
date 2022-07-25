#include "Solver.hpp"

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "env/Map.hpp"

#define THIS_BODY m_rocket.bodies[m_currentBodyIndex]
#define THIS_BODY_SPEC m_rocketSpec.bodySpec(m_currentBodyIndex)

std::shared_ptr<SimuResultLogger> Solver::solve(double windSpeed, double windDirection) {
    switch (AppSetting::WindModel::type) {
    case WindModelType::Real:
        m_windModel = std::make_unique<WindModel>(m_mapData.magneticDeclination);
        break;

    default:
        m_windModel = std::make_unique<WindModel>(windSpeed, windDirection, m_mapData.magneticDeclination);
        break;
    }

    if (m_windModel == nullptr) {
        CommandLine::PrintInfo(PrintInfoType::Error, "Cannot create wind model");
        return nullptr;
    }

    // initialize result
    m_resultLogger = std::make_shared<SimuResultLogger>(m_rocketSpec, m_mapData, windSpeed, windDirection);
    m_resultLogger->pushBody();

    // loop until all rockets are solved
    // single rocket: solve once
    // multi rokcet : every rockets including after detachment
    size_t solvedBodyCount = 0;
    do {
        m_steps = 0;

        initializeRocket();

        // loop until the rocket lands
        while (THIS_BODY.pos.z > 0.0 || THIS_BODY.elapsedTime < 0.1) {
            update();

            if (m_trajectoryMode == TrajectoryMode::Parachute) {
                updateParachute();
            }

            if (m_rocketType == RocketType::Multi && updateDetachment()) {
                break;
            }

            updateAerodynamicParameters();

            updateRocketProperties();

            updateExternalForce();

            updateRocketDelta();

            applyDelta();

            if (m_steps % AppSetting::Result::stepSaveInterval == 0) {
                organizeResult();
            }

            m_steps++;
        }

        // Save last if need
        if (m_steps > 0 && (m_steps - 1) % AppSetting::Result::stepSaveInterval != 0) {
            organizeResult();
        }

        m_resultLogger->setBodyFinalPosition(m_currentBodyIndex, THIS_BODY.pos);

        solvedBodyCount++;

    } while (solvedBodyCount < 2 * m_detachCount + 1);

    return m_resultLogger;
}

void Solver::initializeRocket() {
    // Second, Third Rocket(Multiple)
    if (m_currentBodyIndex < 2 * m_detachCount) {
        nextRocket();
        return;
    }

    // First rocket
    m_bodyDelta.mass       = m_rocketSpec.bodySpec(0).massInitial;
    m_bodyDelta.reflLength = m_rocketSpec.bodySpec(0).CGLengthInitial;
    m_bodyDelta.iyz        = m_rocketSpec.bodySpec(0).rollingMomentInertiaInitial;
    m_bodyDelta.ix         = 0.02;
    m_bodyDelta.pos        = Vector3D(0, 0, 0);
    m_bodyDelta.velocity   = Vector3D(0, 0, 0);
    m_bodyDelta.omega_b    = Vector3D(0, 0, 0);
    m_bodyDelta.quat =
        Quaternion(m_environment.railElevation, (-m_environment.railAzimuth + 90) - m_mapData.magneticDeclination);

    THIS_BODY = m_bodyDelta;
}

void Solver::update() {
    m_windModel->update(THIS_BODY.pos.z);
}

void Solver::updateParachute() {
    const bool detectpeakConditon = m_maxAltitude > THIS_BODY.pos.z + AppSetting::Simulation::detectPeakThreshold;

    if (detectpeakConditon && !THIS_BODY.detectPeak) {
        THIS_BODY.detectPeak = true;
    }

    if (THIS_BODY.parachuteOpened) {
        return;
    }

    const bool detectpeak = THIS_BODY_SPEC.parachutes[0].openingType == ParachuteOpeningType::DetectPeak;

    const bool fixedtime          = THIS_BODY_SPEC.parachutes[0].openingType == ParachuteOpeningType::FixedTime;
    const bool fixedtimeCondition = THIS_BODY.elapsedTime > THIS_BODY_SPEC.parachutes[0].openingTime;

    const bool time_from_detect_peak =
        THIS_BODY_SPEC.parachutes[0].openingType == ParachuteOpeningType::TimeFromDetectPeak;

    if ((detectpeak && detectpeakConditon) || (fixedtime && fixedtimeCondition)) {
        THIS_BODY.parachuteOpened = true;
    }

    const bool time_from_detect_peakCondition =
        THIS_BODY.elapsedTime - m_detectPeakTime > THIS_BODY_SPEC.parachutes[0].openingTime;

    if (time_from_detect_peak) {
        if (!THIS_BODY.waitForOpenPara && detectpeakConditon) {
            THIS_BODY.waitForOpenPara = true;
        }
        if (THIS_BODY.waitForOpenPara && time_from_detect_peakCondition) {
            THIS_BODY.parachuteOpened = true;
            THIS_BODY.waitForOpenPara = false;
        }
    }
}

bool Solver::updateDetachment() {
    bool detachCondition = false;

    switch (m_detachType) {
    case DetachType::BurningFinished:
        detachCondition = THIS_BODY_SPEC.engine.didCombustion(THIS_BODY.elapsedTime);
        break;
    case DetachType::Time:
        detachCondition = THIS_BODY.elapsedTime >= m_detachTime;
        break;
    case DetachType::SyncPara:
        detachCondition = THIS_BODY.parachuteOpened == true;
        break;
    case DetachType::DoNotDeatch:
        return false;
    }

    if (detachCondition && m_detachCount < 1) {  // if need rocket4, 5, 6... , this code should be changed
        // initialize separated bodies
        {
            Body detach;
            detach.ix       = 0.02;
            detach.pos      = THIS_BODY.pos;
            detach.velocity = THIS_BODY.velocity;
            detach.omega_b  = Vector3D();
            detach.quat     = THIS_BODY.quat;

            Body& nextBody1      = m_rocket.bodies[m_currentBodyIndex + 1];
            nextBody1            = detach;
            nextBody1.mass       = m_rocketSpec.bodySpec(m_currentBodyIndex + 1).massInitial;
            nextBody1.reflLength = m_rocketSpec.bodySpec(m_currentBodyIndex + 1).CGLengthInitial;
            nextBody1.iyz        = m_rocketSpec.bodySpec(m_currentBodyIndex + 1).rollingMomentInertiaInitial;

            // receive power from the engine of the upper body for 0.2 seconds
            /*double sumThrust = 0;
            for (double t = 0; t <= 0.2; t += m_dt) {
                sumThrust += m_rocketSpec.bodySpec[m_currentBodyIndex + 2].engine.thrustAt(t) * (0.2 - t) / 0.2;
            }
            nextBody1.velocity -= Vector3D((sumThrust / nextBody1.mass) * m_dt, 0, 0).applyQuaternion(nextBody1.quat);*/

            Body& nextBody2      = m_rocket.bodies[m_currentBodyIndex + 2];
            nextBody2            = detach;
            nextBody2.mass       = m_rocketSpec.bodySpec(m_currentBodyIndex + 2).massInitial;
            nextBody2.reflLength = m_rocketSpec.bodySpec(m_currentBodyIndex + 2).CGLengthInitial;
            nextBody2.iyz        = m_rocketSpec.bodySpec(m_currentBodyIndex + 2).rollingMomentInertiaInitial;
        }

        m_detachCount++;

        return true;
    }

    return false;
}

void Solver::updateAerodynamicParameters() {
    if ((THIS_BODY.velocity - m_windModel->wind()).length() != 0) {
        THIS_BODY.airSpeed_b = (THIS_BODY.velocity - m_windModel->wind()).applyQuaternion(THIS_BODY.quat.conjugated());
    } else {
        THIS_BODY.airSpeed_b = Vector3D();
    }

    THIS_BODY.attackAngle =
        atan(sqrt(THIS_BODY.airSpeed_b.y * THIS_BODY.airSpeed_b.y + THIS_BODY.airSpeed_b.z * THIS_BODY.airSpeed_b.z)
             / (THIS_BODY.airSpeed_b.x + 1e-16));

    THIS_BODY.aeroCoef =
        THIS_BODY_SPEC.aeroCoefStorage.valuesIn(THIS_BODY.airSpeed_b.length(),
                                                THIS_BODY.attackAngle,
                                                THIS_BODY_SPEC.engine.didCombustion(THIS_BODY.elapsedTime));

    const double alpha = atan(THIS_BODY.airSpeed_b.z / (THIS_BODY.airSpeed_b.x + 1e-16));
    const double beta  = atan(THIS_BODY.airSpeed_b.y / (THIS_BODY.airSpeed_b.x + 1e-16));

    THIS_BODY.Cnp = THIS_BODY.aeroCoef.Cna * alpha;
    THIS_BODY.Cny = THIS_BODY.aeroCoef.Cna * beta;

    THIS_BODY.Cmqp = THIS_BODY_SPEC.Cmq;
    THIS_BODY.Cmqy = THIS_BODY_SPEC.Cmq;
}

void Solver::updateRocketProperties() {
    if (THIS_BODY_SPEC.engine.isCombusting(THIS_BODY.elapsedTime)) {
        m_bodyDelta.mass =
            (THIS_BODY_SPEC.massFinal - THIS_BODY_SPEC.massInitial) / THIS_BODY_SPEC.engine.combustionTime();
        m_bodyDelta.reflLength =
            (THIS_BODY_SPEC.CGLengthFinal - THIS_BODY_SPEC.CGLengthInitial) / THIS_BODY_SPEC.engine.combustionTime();
        m_bodyDelta.iyz = (THIS_BODY_SPEC.rollingMomentInertiaFinal - THIS_BODY_SPEC.rollingMomentInertiaInitial)
                          / THIS_BODY_SPEC.engine.combustionTime();
        m_bodyDelta.ix = (0.02 - 0.01) / 3;
    } else {
        m_bodyDelta.mass       = 0;
        m_bodyDelta.reflLength = 0;
        m_bodyDelta.iyz        = 0;
        m_bodyDelta.ix         = 0;
    }
}

void Solver::updateExternalForce() {
    THIS_BODY.force_b  = Vector3D(0, 0, 0);
    THIS_BODY.moment_b = Vector3D(0, 0, 0);

    // Thrust
    THIS_BODY.force_b.x += THIS_BODY_SPEC.engine.thrustAt(THIS_BODY.elapsedTime, m_windModel->pressure());

    if (!THIS_BODY.parachuteOpened) {
        // Aero
        const double preForceCalc = 0.5 * m_windModel->density() * THIS_BODY.airSpeed_b.length()
                                    * THIS_BODY.airSpeed_b.length() * THIS_BODY_SPEC.bottomArea;
        THIS_BODY.force_b.x -= THIS_BODY.aeroCoef.Cd * preForceCalc * cos(THIS_BODY.attackAngle);
        THIS_BODY.force_b.y -= THIS_BODY.Cny * preForceCalc;
        THIS_BODY.force_b.z -= THIS_BODY.Cnp * preForceCalc;

        // Moment
        const double preMomentCalc = 0.25 * m_windModel->density() * THIS_BODY.airSpeed_b.length()
                                     * THIS_BODY_SPEC.length * THIS_BODY_SPEC.length * THIS_BODY_SPEC.bottomArea;
        THIS_BODY.moment_b.x = 0;
        THIS_BODY.moment_b.y = preMomentCalc * THIS_BODY.Cmqp * THIS_BODY.omega_b.y;
        THIS_BODY.moment_b.z = preMomentCalc * THIS_BODY.Cmqy * THIS_BODY.omega_b.z;

        THIS_BODY.moment_b.y += THIS_BODY.force_b.z * (THIS_BODY.aeroCoef.Cp - THIS_BODY.reflLength);
        THIS_BODY.moment_b.z -= THIS_BODY.force_b.y * (THIS_BODY.aeroCoef.Cp - THIS_BODY.reflLength);

        // Gravity
        THIS_BODY.force_b +=
            Vector3D(0, 0, -m_windModel->gravity()).applyQuaternion(THIS_BODY.quat.conjugated()) * THIS_BODY.mass;
    }
}

void Solver::updateRocketDelta() {
    if (THIS_BODY.pos.length() <= m_environment.railLength && THIS_BODY.velocity.z >= 0.0) {  // launch
        if (THIS_BODY.force_b.x < 0) {
            m_bodyDelta.pos      = Vector3D();
            m_bodyDelta.velocity = Vector3D();
            m_bodyDelta.omega_b  = Vector3D();
            m_bodyDelta.quat     = Quaternion();
        } else {
            THIS_BODY.force_b.y = 0;
            THIS_BODY.force_b.z = 0;
            m_bodyDelta.pos     = THIS_BODY.velocity;

            m_bodyDelta.velocity = THIS_BODY.force_b.applyQuaternion(THIS_BODY.quat) / THIS_BODY.mass;

            m_bodyDelta.omega_b = Vector3D();
            m_bodyDelta.quat    = Quaternion();
        }
    } else if (THIS_BODY.parachuteOpened) {  // parachute opened
        const Vector3D paraSpeed = THIS_BODY.velocity;
        const double drag        = 0.5 * m_windModel->density() * paraSpeed.z * paraSpeed.z * 1.0
                            * THIS_BODY_SPEC.parachutes[THIS_BODY.parachuteIndex].Cd;

        m_bodyDelta.velocity.z = drag / THIS_BODY.mass - m_windModel->gravity();
        m_bodyDelta.velocity.x = 0;
        m_bodyDelta.velocity.y = 0;

        THIS_BODY.velocity.x = m_windModel->wind().x;
        THIS_BODY.velocity.y = m_windModel->wind().y;

        m_bodyDelta.pos = THIS_BODY.velocity;

        m_bodyDelta.omega_b = Vector3D();
        m_bodyDelta.quat    = Quaternion();
    } else if (THIS_BODY.pos.z < -10) {  // stop simulation
        m_bodyDelta.velocity = Vector3D();
    } else {  // flight
        if (!m_rocket.launchClear) {
            m_rocket.launchClear = true;
            m_resultLogger->setLaunchClear(THIS_BODY);
        }

        m_bodyDelta.pos      = THIS_BODY.velocity;
        m_bodyDelta.velocity = THIS_BODY.force_b.applyQuaternion(THIS_BODY.quat) / THIS_BODY.mass;

        m_bodyDelta.omega_b.x = THIS_BODY.moment_b.x / THIS_BODY.ix;
        m_bodyDelta.omega_b.y = THIS_BODY.moment_b.y / THIS_BODY.iyz;
        m_bodyDelta.omega_b.z = THIS_BODY.moment_b.z / THIS_BODY.iyz;

        m_bodyDelta.quat = THIS_BODY.quat.angularVelocityApplied(m_bodyDelta.omega_b);
    }
}

void Solver::applyDelta() {
    // Update rocket
    THIS_BODY.mass += m_bodyDelta.mass * m_dt;
    THIS_BODY.reflLength += m_bodyDelta.reflLength * m_dt;
    THIS_BODY.iyz += m_bodyDelta.iyz * m_dt;
    THIS_BODY.ix += m_bodyDelta.ix * m_dt;
    THIS_BODY.pos += m_bodyDelta.pos * m_dt;
    THIS_BODY.velocity += m_bodyDelta.velocity * m_dt;
    THIS_BODY.omega_b += m_bodyDelta.omega_b * m_dt;
    THIS_BODY.quat += m_bodyDelta.quat * m_dt;

    THIS_BODY.quat = THIS_BODY.quat.normalized();

    THIS_BODY.elapsedTime += m_dt;
    m_rocket.timeFromLaunch += m_dt;
}

void Solver::organizeResult() {
    m_resultLogger->update(m_currentBodyIndex,
                           m_rocket,
                           THIS_BODY,
                           *m_windModel.get(),
                           THIS_BODY_SPEC.engine.isCombusting(THIS_BODY.elapsedTime));

    if (m_maxAltitude < THIS_BODY.pos.z) {
        m_maxAltitude    = THIS_BODY.pos.z;
        m_detectPeakTime = THIS_BODY.elapsedTime;
    }
}

void Solver::nextRocket() {
    m_currentBodyIndex++;
    m_resultLogger->pushBody();
}
