#pragma once
#include "Rocket.h"
#include "SpecJson.h"
#include "Engine.h"
#include "Air.h"


enum class TrajectoryMode : int {
	Trajectory = 1,
	Parachute
};

enum class RocketType : int {
	Single = 1,
	Multi
};

enum class DetachType : int {
	BurningFinished = 1,
	Time,
	SyncPara,
	DoNotDeatch
};


struct ResultOfEachRocket {
	//all flight data
	std::vector<Rocket> flightData;

	//max, min values
	double maxHeight = 0.0;
	double maxVelocity = 0.0;
	double maxAttackAngle = 0.0;
	double maxNormalForce = 0.0;

	//special values
	double detectPeakTime = 0.0;
	double timeAtParaOpened = 0.0;
	double heightAtParaOpened = 0.0;
	double airVelAtParaOpened = 0.0;
	double terminalVelocity = 0.0;
	double terminalTime = 0.0;

	//pos
	double lenFromLaunchPoint = 0.0;
	double latitude = 0.0;
	double longitude = 0.0;

	ResultOfEachRocket() {
		flightData.reserve(80000);
	}
};

struct SolvedResult{
	std::vector<ResultOfEachRocket> rocket = std::vector<ResultOfEachRocket>(1);

	//special values
	double windSpeed = 0.0;
	double windDirection = 0.0;
	double launchClearVelocity_ = 0.0;
};

class Solver {

	//config
	const double dt_;
	const RocketType rocketType_;
	const TrajectoryMode trajectoryMode_;
	const DetachType detachType_;
	const double detachTime_;

	//spec
	const SpecJson specJson_;
	std::vector<Engine> engine_;

	//rocket
	Rocket rocket_, rocketDelta_;
	std::vector<Rocket> rocketAtDetached_;

	//dynamics
	Air *air_ = nullptr;
	Vector3D force_b_;
	Vector3D moment_b_;

	//stastus
	bool launchClear_ = false;
	double combustionTime_ = 0.0;
	size_t targetRocketIndex_ = 0;
	size_t usingEngineIndex_ = 0;

	//result
	SolvedResult result_;

public:
	Solver(double dt, RocketType rocketType, TrajectoryMode mode, DetachType detachType,
		double detachTime, const SpecJson& spec, const std::vector<Engine>& engine)
		:dt_(dt),
		rocketType_(rocketType),
		trajectoryMode_(mode),
		detachType_(detachType),
		detachTime_(detachTime),
		specJson_(spec),
		engine_(engine)
	{
	}

	~Solver() {
		delete air_;
	}

	bool run(double windSpeed, double windDirection);

	SolvedResult getResult()const {
		return result_;
	}

private:

	void initializeParameters();

	void update();

	void updateParachuteStatus();

	void updateDetachedStatus();

	void updateParameters();

	void calcDynamicForce();

	void updateDelta();

	void finalUpdate();

	void organizeResult();
};