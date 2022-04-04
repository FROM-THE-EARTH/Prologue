#pragma once

#include <boost/property_tree/ptree.hpp>
#include <string>
#include <vector>

#include "AeroCoefficient.hpp"
#include "Engine.hpp"

enum ParaOpenType : int {
    DetectPeak = 0,
    FixedTime,
    TimeFromDetectPeak,
};

struct Parachute {
    double terminalVelocity;
    ParaOpenType openingType;
    double openingTime;
    double delayTime;
    double openingHeight;

    double Cd = 0;
};

struct RocketParam {
    double length;      // m
    double diameter;    // m
    double bottomArea;  // m^2

    double CGLengthInitial;  // m
    double CGLengthFinal;    // m

    double massInitial;  // kg
    double massFinal;    // kg

    double rollingMomentInertiaInitial;  // kg*m^2
    double rollingMomentInertiaFinal;    // kg*m^2

    double Cmq;

    std::vector<Parachute> parachute;

    Engine engine;
    AeroCoefVsAirspeed aeroCoef;
};

struct RocketSpec {
private:
    bool m_existInfCd = false;

public:
    std::vector<RocketParam> rocketParam;  // could be multiple(multiple rocket)

    void initialize(const std::string& filename);

    static bool IsMultipleRocket(const std::string& filename);

private:
    void setRocketParam(const boost::property_tree::ptree& pt, size_t index);

    void setInfParachuteCd();
};
