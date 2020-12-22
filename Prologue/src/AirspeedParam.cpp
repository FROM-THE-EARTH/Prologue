#include "AirspeedParam.h"

#include "Algorithm.h"

#include <fstream>

size_t search(const std::vector<VsAirspeed>& vs, double airSpeed, size_t begin, size_t end) {
	if (begin == end
		|| begin == end - 1) {
		return begin;
	}

	size_t mid = begin + (end - begin) / 2;
	if (vs[mid].airSpeed == airSpeed) {
		return mid;
	}
	else if (vs[mid].airSpeed > airSpeed) {
		return search(vs, airSpeed, begin, mid);
	}
	else {
		return search(vs, airSpeed, mid, end - 1);
	}
}

size_t getLowerIndex(const std::vector<VsAirspeed>& vs, double airSpeed) {
	size_t index = vs.size() / 2;
	if (vs[index].airSpeed == airSpeed) {
		return index;
	}
	else if (vs[index].airSpeed > airSpeed) {
		return search(vs, airSpeed, 0, index);
	}
	else {
		return search(vs, airSpeed, index, vs.size() - 1);
	}
}

void AirspeedParam::loadParam(const std::string& filename) {
	std::fstream fs("input/airspeed_param/" + filename);

	if (!fs.is_open()) {
		return;
	}

	char header[1024];
	fs.getline(header, 1024);
	size_t i = 0;
	char c;
	std::string dummy;
	while (!fs.eof()) {
		vsAirspeed_.push_back(VsAirspeed());
		fs >> vsAirspeed_[i].airSpeed >> c
			>> vsAirspeed_[i].Cp >> c
			>> vsAirspeed_[i].Cp_a >> c
			>> vsAirspeed_[i].Cd >> c
			>> vsAirspeed_[i].Cd_a2 >> c
			>> vsAirspeed_[i].Cna;
		i++;
	}
	if (vsAirspeed_[vsAirspeed_.size() - 1] == VsAirspeed()) {
		vsAirspeed_.pop_back();
	}

	fs.close();

	exist_ = true;
}

void AirspeedParam::update(double airSpeed) {
	if (vsAirspeed_.size() == 1) {
		param_ = {
			airSpeed,
			vsAirspeed_[0].Cp,
			vsAirspeed_[0].Cp_a,
			vsAirspeed_[0].Cd,
			vsAirspeed_[0].Cd_a2,vsAirspeed_[0].Cna
		};
		return;
	}

	const size_t i = getLowerIndex(vsAirspeed_, airSpeed);

	const double airSpeed1 = vsAirspeed_[i].airSpeed;
	const double airSpeed2 = vsAirspeed_[i + 1].airSpeed;

	if (airSpeed < airSpeed1) {
		param_ = {
			airSpeed,
			vsAirspeed_[i].Cp,
			vsAirspeed_[i].Cp_a,
			vsAirspeed_[i].Cd,
			vsAirspeed_[i].Cd_a2,
			vsAirspeed_[i].Cna
		};
		return;
	}

	if (airSpeed > airSpeed2) {
		param_ = {
			airSpeed,
			vsAirspeed_[i + 1].Cp,
			vsAirspeed_[i + 1].Cp_a,
			vsAirspeed_[i + 1].Cd,
			vsAirspeed_[i + 1].Cd_a2,
			vsAirspeed_[i + 1].Cna
		};
		return;
	}

	param_ = {
		airSpeed,
		Algorithm::ToLinear(airSpeed, airSpeed1, airSpeed2, vsAirspeed_[i].Cp, vsAirspeed_[i + 1].Cp),
		Algorithm::ToLinear(airSpeed, airSpeed1, airSpeed2, vsAirspeed_[i].Cp_a, vsAirspeed_[i + 1].Cp_a),
		Algorithm::ToLinear(airSpeed, airSpeed1, airSpeed2, vsAirspeed_[i].Cd, vsAirspeed_[i + 1].Cd),
		Algorithm::ToLinear(airSpeed, airSpeed1, airSpeed2, vsAirspeed_[i].Cd_a2, vsAirspeed_[i + 1].Cd_a2),
		Algorithm::ToLinear(airSpeed, airSpeed1, airSpeed2, vsAirspeed_[i].Cna, vsAirspeed_[i + 1].Cna)
	};
}