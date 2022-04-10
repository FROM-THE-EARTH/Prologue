#pragma once

#include "AeroCoefficient.hpp"
#include "math/Quaternion.hpp"
#include "math/Vector3D.hpp"

// Parameters of the part under simulation
// subscript "_b" represent the variable is based on body coordinate
struct Body {
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
    AeroCoefficient aeroCoef;
    double Cnp, Cny;
    double Cmqp, Cmqy;
    Vector3D force_b;
    Vector3D moment_b;

    // status
    double elapsedTime    = 0.0;  // [s]
    size_t parachuteIndex = 0;
    bool parachuteOpened  = false;
    bool waitForOpenPara  = false;
    bool detectPeak       = false;

    // calculated
    Vector3D airSpeed_b;  // [m/s]
    double attackAngle;   // [rad]

    /*===========================result===========================*/
    double lenFromLaunchPoint = 0.0;
    double latitude           = 0.0;
    double longitude          = 0.0;
};

struct Rocket {
    // rocket1, rocket2, rocket3, ...
    std::vector<Body> bodies;

    double timeFromLaunch = 0.0;  // [s]
    bool launchClear      = false;
};
