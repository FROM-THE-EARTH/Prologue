#pragma once

#include <string>
#include <vector>

struct AeroCoefficient {
    // These vars are used only calculation.
    // Do not refer to these
    struct InternalVars {
        double airspeed = 0;
        double Cd_i     = 0;
        double Cd_f     = 0;
        double Cp_a     = 0;
        double Cd_a2    = 0;
    };

    double Cp  = 0;
    double Cd  = 0;
    double Cna = 0;

    InternalVars internalVars;
};

class AeroCoefficientStorage {
    std::vector<AeroCoefficient> m_aeroCoefs;

    bool m_exist = false;

public:
    bool exist() const {
        return m_exist;
    }

    AeroCoefficient valuesIn(double airspeed, double attackAngle, bool combustionEnded) const;

    // initialize parameters by csv file
    void init(const std::string& filename);

    void init(double Cp, double Cna, double Cd_i, double Cd_f, double Cp_a, double Cd_a2) {
        m_aeroCoefs.push_back({Cp, 0.0, Cna, {0.0, Cd_i, Cd_f, Cp_a, Cd_a2}});
    }
};
