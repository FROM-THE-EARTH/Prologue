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
};

class AirspeedParam{

	std::vector<VsAirspeed> vsAirspeed_;

	bool exist_ = false;

public:
	void loadParam(const std::string filename);

	bool exist() const {
		return exist_;
	}

	VsAirspeed getParam(double airSpeed);
};