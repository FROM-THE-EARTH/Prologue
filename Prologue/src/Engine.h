#pragma once
#include <string>
#include <vector>

struct ThrustData {
	double time = 0.0;
	double thrust = 0.0;
};

class Engine {

	std::vector<ThrustData> thrustDatas_;

	bool isNull_ = false;

public:

	bool loadThrustData(const std::string& filename);

	double thrustAt(double time) const;

	double combustionTime()const {
		return isNull_ ? 0.0 : thrustDatas_[thrustDatas_.size() - 1].time;
	}

	bool isFinishBurning(double time) const {
		return isNull_ ? true : time > combustionTime();
	}
};