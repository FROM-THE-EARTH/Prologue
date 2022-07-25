// ------------------------------------------------
// ロケットの諸元に関するデータクラス
// ------------------------------------------------

#pragma once

#include <boost/property_tree/ptree.hpp>
#include <string>
#include <vector>

#include "AeroCoefficient.hpp"
#include "Engine.hpp"

enum class ParachuteOpeningType : size_t {
    DetectPeak = 0,
    FixedTime,
    TimeFromDetectPeak,
};

struct Parachute {
    ParachuteOpeningType openingType;
    double terminalVelocity;
    double openingTime;
    double delayTime;
    double openingHeight;

    double Cd = 0;
};

struct BodySpecification {
    double length;      // [m]
    double diameter;    // [m]
    double bottomArea;  // [m^2]

    double CGLengthInitial;  // [m]
    double CGLengthFinal;    // [m]

    double massInitial;  // [kg]
    double massFinal;    // [kg]

    double rollingMomentInertiaInitial;  // [kg*m^2]
    double rollingMomentInertiaFinal;    // [kg*m^2]

    double Cmq;

    std::vector<Parachute> parachutes;

    Engine engine;
    AeroCoefficientStorage aeroCoefStorage;
};

class RocketSpecification {
private:
    std::vector<BodySpecification> m_bodySpecs;
    bool m_existInfCd = false;

public:
    RocketSpecification() = delete;

    explicit RocketSpecification(const std::string& specFileName);

    static bool IsMultipleRocket(const std::string& specFileName);

    size_t bodyCount() const {
        return m_bodySpecs.size();
    }

    const BodySpecification& bodySpec(size_t bodyIndex) const {
        return m_bodySpecs[bodyIndex];
    }

private:
    void setBodySpecification(const boost::property_tree::ptree& pt, size_t index);

    void setInfParachuteCd();
};
