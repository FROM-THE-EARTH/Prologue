#pragma once

#include <string>
#include <vector>

struct VsAirspeed {
    double airSpeed;
    double Cp;
    double Cp_a;
    double Cd;
    double Cd_a2;
    double Cna;

    bool operator==(const VsAirspeed& v) {
        return (airSpeed == v.airSpeed && Cp == v.Cp && Cp_a == v.Cp_a && Cd == v.Cd && Cd_a2 == v.Cd_a2
                && Cna == v.Cna);
    }
};

class AirspeedParam {
    std::vector<VsAirspeed> m_vsAirspeed;

    VsAirspeed m_param;

    bool m_exist = false;

public:
    void loadParam(const std::string& filename);

    bool exist() const {
        return m_exist;
    }

    void update(double airSpeed);

    VsAirspeed getParam() const {
        return m_param;
    }

    void setParam(double Cp, double Cp_a, double Cd, double Cd_a2, double Cna) {
        const VsAirspeed v = {0.0, Cp, Cp_a, Cd, Cd_a2, Cna};
        m_vsAirspeed.push_back(v);
    }
};
