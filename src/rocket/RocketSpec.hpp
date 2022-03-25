#pragma once

#include <boost/property_tree/ptree.hpp>
#include <string>
#include <vector>

#include "AirspeedParam.hpp"
#include "Engine.hpp"

struct Information {
    std::string teamName;
    std::string rocketName;
    std::string experimentDate;
    std::string version;
};

struct Parachute {
    double terminalVelocity;
    int openingType;
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
    AirspeedParam airspeedParam;
};

struct Environment {
    std::string place;
    double railLength;
    double railAzimuth;
    double railElevation;
};

struct RocketSpec {
private:
    bool m_existInfCd = false;

public:
    Information info;
    std::vector<RocketParam> rocketParam;  // could be multiple(multiple rocket)
    Environment env;

    void initialize(const std::string& filename);

    static bool IsMultipleRocket(const std::string& filename);

private:
    void setEnvironment(const boost::property_tree::ptree& pt);

    void setExtraInfo(const boost::property_tree::ptree& pt);

    void setRocketParam(const boost::property_tree::ptree& pt, size_t index);

    void setInfParachuteCd();
};
