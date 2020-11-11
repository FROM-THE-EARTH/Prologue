#include "Engine.h"

#include <iostream>
#include <fstream>
#include <algorithm>

size_t search(const std::vector<ThrustData>& thrust, double time, size_t begin, size_t end) {
	if (begin == end
		|| begin == end - 1) {
		return begin;
	}

	size_t mid = begin + (end - begin) / 2;
	if (thrust[mid].time == time) {
		return mid;
	}
	else if (thrust[mid].time > time) {
		return search(thrust, time, begin, mid);
	}
	else {
		return search(thrust, time, mid, end - 1);
	}
}

size_t getLowerIndex(const std::vector<ThrustData>& thrust, double time) {
	size_t index = thrust.size() / 2;
	if (thrust[index].time == time) {
		return index;
	}
	else if (thrust[index].time > time) {
		return search(thrust, time, 0, index);
	}
	else {
		return search(thrust, time, index, thrust.size() - 1);
	}
}

bool Engine::loadThrustData(const std::string& filename) {
	std::fstream fs("input/thrust/" + filename);

	if (!fs.is_open()) {
		return false;
	}

	char dummy[1024];

	while (1) {
		char c = fs.get();
		if (c < '0' || c > '9') {
			fs.getline(dummy, 1024);
		}
		else {
			fs.unget();
			break;
		}
	}

	while (!fs.eof()) {
		ThrustData thrustdata;
		fs >> thrustdata.time >> thrustdata.thrust;
		thrustDatas_.push_back(thrustdata);
	}

	if (thrustDatas_[thrustDatas_.size() - 1].time == 0.0) {
		thrustDatas_.pop_back();
	}

	if (thrustDatas_[0].time != 0.0) {
		thrustDatas_.insert(thrustDatas_.begin(), ThrustData{ 0.0,0.0 });
	}

	return true;
}

double Engine::thrustAt(double time) {
	if (time <= 0.0 || time > thrustDatas_[thrustDatas_.size() - 1].time) {
		return 0;
	}

	const size_t i = getLowerIndex(thrustDatas_, time);

	const double time1 = thrustDatas_[i - 1].time;
	const double time2 = thrustDatas_[i].time;
	const double thrust1 = thrustDatas_[i - 1].thrust;
	const double thrust2 = thrustDatas_[i].thrust;

	const double grad = (thrust2 - thrust1) / (time2 - time1);

	const double thrust = thrust1 + grad * (time - time1);

	return thrust;
}