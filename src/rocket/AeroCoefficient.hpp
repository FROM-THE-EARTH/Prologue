#pragma once

#include <string>
#include <vector>

struct AeroCoefficient {
    struct InternalVars {
        double airspeed;
        double Cp_a;
        double Cd_a2;
    };

    double Cp;
    double Cd;
    double Cna;

    InternalVars internalVars;

    bool operator==(const AeroCoefficient& other) const {
        return (Cp == other.Cp && Cd == other.Cd && Cna == other.Cna
                && internalVars.airspeed == other.internalVars.airspeed && internalVars.Cp_a == other.internalVars.Cp_a
                && internalVars.Cd_a2 == other.internalVars.Cd_a2);
    }
};

class AeroCoefficientStorage {
    std::vector<AeroCoefficient> m_aeroCoefs;

    bool m_exist = false;

public:
    bool exist() const {
        return m_exist;
    }

    AeroCoefficient valuesIn(double airspeed, double attackAngle) const;

    // initialize parameters by csv file
    void init(const std::string& filename);

    void setParam(double Cp, double Cp_a, double Cd, double Cd_a2, double Cna) {
        const AeroCoefficient coef = {Cp, Cd, Cna, {0.0, Cp_a, Cd_a2}};
        m_aeroCoefs.push_back(coef);
    }
};
