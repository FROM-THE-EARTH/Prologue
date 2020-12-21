#pragma once
#pragma once
#include <string>
#include <vector>
#include "Engine.h"


struct Information {
	std::string teamName;
	std::string rocketName;
	std::string experimentDate;
	std::string version;
};

struct Parachute {
	double terminalVelocity;
	int openingType;
	double openingTime;
	double delayTime;
	double openingHeight;

	double Cd = 0;
};

struct RocketParam {
	double length;//m
	double diameter;//m
	double bottomArea;//m^2

	double CGLengthInitial;//m
	double CGLengthFinal;//m

	double massInitial;//kg
	double massFinal;//kg

	double rollingMomentInertiaInitial;//kg*m^2
	double rollingMomentInertiaFinal;//kg*m^2

	double CPLength;//m

	double Cd;

	double Cna;

	double Cmq;

	std::vector<Parachute> parachute;

	Engine engine;
};

struct Environment {
	std::string place;
	double railLength;
	double railAzimuth;
	double railElevation;
};


struct RocketSpec {
	Information info;
	std::vector <RocketParam> rocketParam;//could be multiple(multiple rocket)
	Environment env;
};