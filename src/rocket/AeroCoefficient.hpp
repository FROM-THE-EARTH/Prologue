#pragma once

#include <string>
#include <vector>

struct AeroCoefficient {
    double airspeed;
    double Cp;
    double Cp_a;
    double Cd;
    double Cd_a2;
    double Cna;

    bool operator==(const AeroCoefficient& other) const {
        return (airspeed == other.airspeed && Cp == other.Cp && Cp_a == other.Cp_a && Cd == other.Cd
                && Cd_a2 == other.Cd_a2 && Cna == other.Cna);
    }
};

class AeroCoefVsAirspeed {
    std::vector<AeroCoefficient> m_aeroCoefs;

    AeroCoefficient m_currentAeroCoef;

    bool m_exist = false;

public:
    // initialize parameters by csv file
    void init(const std::string& filename);

    bool exist() const {
        return m_exist;
    }

    void update(double airspeed);

    AeroCoefficient get() const {
        return m_currentAeroCoef;
    }

    void setParam(double Cp, double Cp_a, double Cd, double Cd_a2, double Cna) {
        const AeroCoefficient coef = {0.0, Cp, Cp_a, Cd, Cd_a2, Cna};
        m_aeroCoefs.push_back(coef);
    }
};
