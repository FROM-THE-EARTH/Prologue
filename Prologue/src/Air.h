#pragma once
#include "Vector3D.h"

#include <string>
#include <vector>


struct WindData {
	double height = 0;
	double speed = 0;
	double direction = 0;

	bool operator==(const WindData& w) {
		return (
			height == w.height
			&& speed == w.speed
			&& direction == w.direction
			);
	}
};


class Air {

	std::vector<WindData> windData_;

	const double groundWindSpeed_, groundWindDirection_;

	double directionInterval_ = 0.0;

	double height_ = 0.0;
	double geopotentialHeight_ = 0.0;
	double gravity_ = 0.0;
	double temperature_ = 0.0;
	double pressure_ = 0.0;
	double airDensity_ = 0.0;
	Vector3D wind_;

	bool initialized_ = false;

public:

	Air();//real

	Air(double groundWindSpeed, double groundWindDirection);//original or only_powerlow

	void update(double height);

	Vector3D wind() const {
		return wind_;
	}

	double density() const {
		return airDensity_;
	}

	double gravity()  const {
		return gravity_;
	}

	bool initialized()const {
		return initialized_;
	}

private:

	double getGeopotentialHeight();

	double getGravity();

	double getTemperature();

	double getPressure();

	double getAirDensity();

	Vector3D getWindFromData();

	Vector3D getWindOriginalModel();

	Vector3D getWindOnlyPowerLow();
};