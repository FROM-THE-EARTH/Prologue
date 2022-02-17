#pragma once

#include "math/Quaternion.hpp"
#include "math/Vector3D.hpp"

// Parameters of the part under simulation
// subscript "_b" represent the variable is based on body coordinate
struct Rocket {
    /*========================delta exists=========================*/
    double mass;        // mass [kg]
    double reflLength;  // length from nose to center of mass[m]
    double iyz;         // inertia moment of pitching & yawing [kg*m^2]
    double ix;          // inertia moment of rolling [kg*m^2]
    Vector3D pos;       // position [m] (ENU coordinate)
    Vector3D velocity;  // velocity [m/s] (ground speed)
    Vector3D omega_b;   // angular velocity (roll,pitch,yaw)
    Quaternion quat;    // quaternion

    /*========================delta not exists=====================*/
    // param
    double Cnp, Cny;
    double Cmqp, Cmqy;

    // status
    double elapsedTime    = 0.0;  // [s]
    size_t parachuteIndex = 0;
    bool parachuteOpened  = false;
    bool waitForOpenPara  = false;
    bool detectPeak       = false;

    // calculated
    Vector3D airSpeed_b;  // [m/s]
    double attackAngle;   // [rad]

    /*============================operator==========================*/
    Rocket& operator+=(const Rocket& r) {
        mass += r.mass;
        reflLength += r.reflLength;
        iyz += r.iyz;
        ix += r.ix;
        pos += r.pos;
        velocity += r.velocity;
        omega_b += r.omega_b;
        quat += r.quat;

        return *this;
    }

    Rocket operator*(double s) {
        return {mass * s,
                reflLength * s,
                iyz * s,
                ix * s,
                pos * s,
                velocity * s,
                omega_b * s,
                quat * s,
                Cnp,
                Cny,
                Cmqp,
                Cmqy,
                elapsedTime,
                parachuteIndex,
                parachuteOpened,
                waitForOpenPara,
                detectPeak,
                airSpeed_b,
                attackAngle};
    }
};
