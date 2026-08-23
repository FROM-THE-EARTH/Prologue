// ------------------------------------------------
// ロケットの諸元に関するデータクラス
// ------------------------------------------------

#pragma once

#include <vector>

#include "AeroCoefficient.hpp"
#include "Engine.hpp"

// Parachute opening type flag
#define PARACHUTE_OPENING_TYPE_DETECT_PEAK           0x01
#define PARACHUTE_OPENING_TYPE_FIXED_TIME            0x02
#define PARACHUTE_OPENING_TYPE_TIME_FROM_DETECT_PEAK 0x04

struct Parachute {
    unsigned char openingType = 0; // PARACHUTE_OPENING_TYPE_*
    double openingTime        = 0;
    double delayTime          = 0;
    double openingHeight      = 0;

    double CdS = 0;
};

struct Transition {
    double time = 0;
    double mass = 0;
    double Cd   = 0;
};

struct BodySpecification {
    double length     = 0;  // [m]
    double diameter   = 0;  // [m]
    double bottomArea = 0;  // [m^2]

    double CGLengthInitial = 0;  // [m]
    double CGLengthFinal   = 0;  // [m]

    double massInitial = 0;  // [kg]
    double massFinal   = 0;  // [kg]

    double rollingMomentInertiaInitial = 0;  // [kg*m^2]
    double rollingMomentInertiaFinal   = 0;  // [kg*m^2]

    double Cmq = 0;

    std::vector<Parachute> parachutes;

    Engine engine;
    AeroCoefficientStorage aeroCoefStorage;

    std::vector<Transition> transitions;
};

struct SeparationSpecification {
    size_t sourceBodyIndex = 0;
    std::vector<size_t> productBodyIndices;
};

class RocketSpecification {
private:
    std::vector<BodySpecification> m_bodySpecs;
    std::vector<SeparationSpecification> m_separations;
public:
    RocketSpecification() = delete;

    explicit RocketSpecification(std::vector<BodySpecification> bodySpecs,
                                 std::vector<SeparationSpecification> separations = {});

    size_t bodyCount() const {
        return m_bodySpecs.size();
    }

    bool isMultiple() const {
        return !m_separations.empty();
    }

    const std::vector<SeparationSpecification>& separations() const {
        return m_separations;
    }

    const BodySpecification& bodySpec(size_t bodyIndex) const {
        return m_bodySpecs[bodyIndex];
    }

    BodySpecification& bodySpec(size_t bodyIndex) {
        return m_bodySpecs[bodyIndex];
    }
};
