#pragma once

#include <string>
#include <vector>

namespace Internal {
    struct AeroCoefSpec {
        double airspeed = 0;
        double Cp       = 0;
        double Cp_a     = 0;
        double Cd_i     = 0;
        double Cd_f     = 0;
        double Cd_a2    = 0;
        double Cna      = 0;
    };
}

struct AeroCoefficient {
    double Cp  = 0;
    double Cd  = 0;
    double Cna = 0;
};

class AeroCoefficientStorage {
    std::vector<Internal::AeroCoefSpec> m_aeroCoefSpec;

    bool m_isTimeSeries = false;

public:
    bool isTimeSeriesSpec() const {
        return m_isTimeSeries;
    }

    AeroCoefficient valuesIn(double airspeed, double attackAngle, bool combustionEnded) const;

    // initialize parameters by csv file
    void init(const std::string& filename);

    void init(double Cp, double Cp_a, double Cd_i, double Cd_f, double Cd_a2, double Cna) {
        m_aeroCoefSpec.emplace_back(Internal::AeroCoefSpec{0, Cp, Cp_a, Cd_i, Cd_f, Cd_a2, Cna});
    }
};
