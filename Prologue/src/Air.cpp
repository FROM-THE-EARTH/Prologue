#include "Air.h"

#include "Constant.h"
#include "AppSetting.h"
#include "CommandLine.h"

#include <fstream>
#include <iostream>


//threshold
constexpr size_t N = 3;
constexpr double heightList[N + 1] = { 0.0, 11000, 20000, 32000 };//height threshold[m]
constexpr double baseTemperature[N]{ 15 - Constant::AbsoluteZero, -56.5 - Constant::AbsoluteZero, -76.5 - Constant::AbsoluteZero };//[kelvin]
constexpr double tempDecayRate[N] = { -6.5e-3, 0.0, 1e-3 };// temperature decay rate
constexpr double basePressure[N] = { Constant::SeaPressure, 22632.064, 5474.9 };//[Pa]


//wind calculation
constexpr double geostrophicWind = 15;//[m/s]
constexpr double surfaceLayerLimit = 300;//[m] Surface layer -300[m]
constexpr double ekmanLayerLimit = 1000;//[m] Ekman layer 300-1000[m]


Air::Air(double _groundWindSpeed, double _groundWindDirection)
	:groundWindSpeed_(_groundWindSpeed),
	groundWindDirection_(_groundWindDirection)
{
	directionInterval_ = 270 - groundWindDirection_;
	if (directionInterval_ <= -45.0) {
		directionInterval_ = 270 - groundWindDirection_ + 360;
	}

	initialized_ = true;
}


Air::Air()
	:groundWindSpeed_(0.0),
	groundWindDirection_(0.0) {
	if (!windData_.empty()) {
		initialized_ = true;
		return;
	}

	std::fstream windfile("input/wind/" + AppSetting::Setting().windModel.realdataFilename);

	size_t i = 1;
	windData_.push_back(WindData());
	char c;
	while (!windfile.eof()) {
		windData_.push_back(WindData());
		windfile >> windData_[i].height >> c >> windData_[i].speed >> c >> windData_[i].direction;
		i++;
	}
	if (windData_[windData_.size() - 1].speed == 0) {
		windData_.pop_back();
	}

	windfile.close();

	if (windData_.size() == 0) {
		initialized_ = false;
		return;
	}

	initialized_ = true;
}


void Air::update(double _height) {
	height_ = _height;

	geopotentialHeight_ = getGeopotentialHeight();
	gravity_ = getGravity();
	temperature_ = getTemperature();
	pressure_ = getPressure();
	airDensity_ = getAirDensity();

	switch (AppSetting::Setting().windModel.type)
	{
	case WindModelType::Real:
		wind_ = getWindFromData();
		break;

	case WindModelType::Original:
		wind_ = getWindOriginalModel();
		break;

	case WindModelType::OnlyPowerLow:
		wind_ = getWindOnlyPowerLow();
		break;
	}
}


double Air::getGeopotentialHeight() {
	return Constant::EarthRadius * height_ / (Constant::EarthRadius + height_);
}


double Air::getGravity() {
	const double k = (Constant::EarthRadius / (Constant::EarthRadius + height_));
	return Constant::G * k * k;
}


double Air::getTemperature() {
	for (size_t i = 0; i < N; i++) {
		if (geopotentialHeight_ < heightList[i + 1])
			return baseTemperature[i] + tempDecayRate[i] * (geopotentialHeight_ - heightList[i]) + Constant::AbsoluteZero;
	}

	CommandLine::PrintInfo(PrintInfoType::Error, "Not defined above 32000m");

	return 0;
}


double Air::getPressure() {

	const double temp = temperature_ - Constant::AbsoluteZero;//[kelvin]

	for (size_t i = 0; i < N; i++) {
		if (geopotentialHeight_ <= heightList[i + 1]) {
			if (i == 1)
				return basePressure[i] * exp(-(geopotentialHeight_ - heightList[i]) * Constant::G / (Constant::GasConstant * baseTemperature[i]));
			else
				return basePressure[i] * pow(baseTemperature[i] / temp, Constant::G / (Constant::GasConstant * tempDecayRate[i]));
		}
	}

	CommandLine::PrintInfo(PrintInfoType::Error, "Not defined above 32000m");

	return 0;
}


double Air::getAirDensity() {
	return  pressure_ / ((temperature_ - Constant::AbsoluteZero) * Constant::GasConstant);
}


Vector3D Air::getWindFromData() {
	size_t index = 0;
	for (size_t i = 0; i < windData_.size(); i++) {
		if (height_ > windData_[i].height) {
			index++;
		}
	}

	if (index == 0) {
		return Vector3D(0, 0, 0);
	}

	const double windSpeed = windData_[index - 1].speed +
		(windData_[index].speed - windData_[index - 1].speed) / (windData_[index].height - windData_[index - 1].height)
		* (height_ - windData_[index - 1].height);

	const double direction = windData_[index - 1].direction +
		(windData_[index].direction - windData_[index - 1].direction) / (windData_[index].height - windData_[index - 1].height)
		* (height_ - windData_[index - 1].height);


	const double rad = direction * Constant::PI / 180;

	return -Vector3D(sin(rad), cos(rad), 0) * windSpeed;
}


Vector3D Air::getWindOriginalModel() {
	if (height_ <= 0) {
		const double rad = groundWindDirection_ * Constant::PI / 180;
		const Vector3D groundWind = -Vector3D(sin(rad), cos(rad), 0) * groundWindSpeed_;

		return groundWind;
	}

	if (height_ < surfaceLayerLimit) {//Surface layer
		const double deltaDirection = height_ / ekmanLayerLimit * directionInterval_;
		const double rad = (groundWindDirection_ + deltaDirection) * Constant::PI / 180;
		const Vector3D wind = -Vector3D(sin(rad), cos(rad), 0) * groundWindSpeed_;

		return wind * pow(height_ * 0.5, 1.0 / AppSetting::Setting().windModel.powerConstant);
	}
	else if (height_ < ekmanLayerLimit) {//Ekman layer
		const double deltaDirection = height_ / ekmanLayerLimit * directionInterval_;
		const double rad = (groundWindDirection_ + deltaDirection) * Constant::PI / 180;

		const double borderWindSpeed = groundWindSpeed_ * pow(height_ * 0.5, 1.0 / AppSetting::Setting().windModel.powerConstant);

		const double k = (height_ - surfaceLayerLimit) / (surfaceLayerLimit * sqrt(2));
		const double u = geostrophicWind * (1 - exp(-k) * cos(k));
		const double v = geostrophicWind * exp(-k) * sin(k);

		const double descentRate = ((geostrophicWind - u) / geostrophicWind);


		return -Vector3D(sin(rad), cos(rad), 0) * borderWindSpeed * descentRate - Vector3D(u * sin(rad), u * cos(rad), v);
	}
	else {//Free atomosphere
		return Vector3D(geostrophicWind, 0, 0);
	}
}


Vector3D Air::getWindOnlyPowerLow() {
	if (height_ <= 0) {
		const double rad = groundWindDirection_ * Constant::PI / 180;
		return -Vector3D(sin(rad), cos(rad), 0) * groundWindSpeed_;
	}
	else {
		const double rad = groundWindDirection_ * Constant::PI / 180;
		const Vector3D wind = -Vector3D(sin(rad), cos(rad), 0) * groundWindSpeed_;

		return wind * pow(height_ * 0.5, 1.0 / AppSetting::Setting().windModel.powerConstant);
	}
}