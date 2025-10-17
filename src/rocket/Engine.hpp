// ------------------------------------------------
// エンジンクラス
// ------------------------------------------------

#pragma once

#include <string>
#include <vector>

#include <algorithm> 

#include "misc/Constant.hpp"

struct ThrustData {
    double time   = 0.0;
    double thrust = 0.0;
};

class Engine {
    std::vector<ThrustData> m_thrustData;

    double m_thrustMeasuredPressure = 101325;  // [Pa]
    double m_nozzleArea             = 0.0;     // [m^2]

    bool m_exist = false;

public:
    bool loadThrustData(const std::string& filename);

    void setThrustMeasuredPressure(double pressure) {
        m_thrustMeasuredPressure = pressure;
    }

    void setNozzleDiameter(double diameter) {
        m_nozzleArea = Constant::PI * diameter * diameter / 4.0;
    }

    double thrustAt(double time, double pressure) const;

    double combustionTime() const {
        return m_exist ? m_thrustData[m_thrustData.size() - 1].time : 0.0;
    }

    bool isCombusting(double time) const {
        return m_exist ? time <= combustionTime() : false;
    }

    bool didCombustion(double time) const {
        return m_exist ? time > combustionTime() : true;
    }
};
