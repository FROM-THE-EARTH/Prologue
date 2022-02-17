#pragma once

#include <string>
#include <vector>

struct ThrustData {
    double time   = 0.0;
    double thrust = 0.0;
};

class Engine {
    std::vector<ThrustData> m_thrustData;

    bool m_exist = false;

public:
    bool loadThrustData(const std::string& filename);

    double thrustAt(double time) const;

    double combustionTime() const {
        return m_exist ? m_thrustData[m_thrustData.size() - 1].time : 0.0;
    }

    bool isFinishBurning(double time) const {
        return m_exist ? time > combustionTime() : true;
    }
};
