#pragma once

#include <string>
#include <vector>

struct ThrustData {
    double time   = 0.0;
    double thrust = 0.0;
};

class Engine {
    std::vector<ThrustData> thrustDatas_;

    bool exist_ = false;

public:
    bool loadThrustData(const std::string& filename);

    double thrustAt(double time) const;

    double combustionTime() const {
        return exist_ ? thrustDatas_[thrustDatas_.size() - 1].time : 0.0;
    }

    bool isFinishBurning(double time) const {
        return exist_ ? time > combustionTime() : true;
    }
};
