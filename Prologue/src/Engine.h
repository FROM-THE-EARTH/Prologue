#pragma once
#include <string>
#include <vector>

struct ThrustData {
	double time = 0.0;
	double thrust = 0.0;
};

class Engine {

	std::vector<ThrustData> thrustDatas_;

public:

	bool loadThrustData(const std::string& filename);

	double thrustAt(double time);

	double combustionTime()const {
		return thrustDatas_[thrustDatas_.size() - 1].time;
	}

	bool isFinishBurning(double time) const {
		return time > combustionTime();
	}
};