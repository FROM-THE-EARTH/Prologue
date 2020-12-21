#include "AirspeedParam.h"

#include <fstream>

void AirspeedParam::loadParam(const std::string filename) {
	std::fstream fs("input/airspeed_param/" + filename);

	if (!fs.is_open()) {
		return;
	}

	size_t i = 1;
	char c;
	std::string dummy;
	fs >> dummy >> c >> dummy >> c >> dummy >> c >> dummy >> c >> dummy >> c >> dummy >> dummy;//read header
	while (!fs.eof()) {
		vsAirspeed_.push_back({});
		fs >> vsAirspeed_[i].airSpeed >> c
			>> vsAirspeed_[i].Cp >> c
			>> vsAirspeed_[i].Cp_a >> c
			>> vsAirspeed_[i].Cd >> c
			>> vsAirspeed_[i].Cd_a2 >> c
			>> vsAirspeed_[i].Cna >> dummy;
		i++;
	}

	fs.close();

	exist_ = true;
}

VsAirspeed AirspeedParam::getParam(double airspeed) {

	return { airspeed,0,0,0,0,0 };
}