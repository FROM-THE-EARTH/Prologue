// ------------------------------------------------
// 対気速度に対する係数の決定のためのクラス
// ------------------------------------------------

#pragma once

#include <vector>

struct AeroCoefficientData {
    double airspeed = 0;
    double Cp       = 0;
    double Cp_a     = 0;
    double Cd_i     = 0;
    double Cd_f     = 0;
    double Cd_a2    = 0;
    double Cna      = 0;
};

struct AeroCoefficient {
    double Cp  = 0;
    double Cd  = 0;
    double Cna = 0;
};

class AeroCoefficientStorage {
    std::vector<AeroCoefficientData> m_aeroCoefSpec;

    AeroCoefficient m_constant;

    bool m_isTimeSeries = false;

public:
    AeroCoefficientStorage() = default;

    explicit AeroCoefficientStorage(std::vector<AeroCoefficientData> aeroCoefSpec, bool isTimeSeries);

    bool isTimeSeriesSpec() const {
        return m_isTimeSeries;
    }

    AeroCoefficient valuesIn(double airspeed, double attackAngle, bool combustionEnded) const;

    void init(double Cp, double Cp_a, double Cd_i, double Cd_f, double Cd_a2, double Cna) {
        m_aeroCoefSpec.emplace_back(AeroCoefficientData{0, Cp, Cp_a, Cd_i, Cd_f, Cd_a2, Cna});
    }

    void setConstant(double Cp, double Cd, double Cna) {
        m_constant = {Cp, Cd, Cna};
    }
};
