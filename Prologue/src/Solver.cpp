#include "Solver.h"

#include "AppSetting.h"
#include "CommandLine.h"
#include "MapFeature.h"


bool Solver::run(double windSpeed, double windDirection) {
	switch (AppSetting::Setting().windModel.type) {
	case WindModelType::Real:
		air_ = new Air();
		break;

	default:
		air_ = new Air(windSpeed, windDirection);
		break;
	}

	if (!air_->initialized()) {
		CommandLine::PrintInfo(PrintInfoType::Error, "Cannot create wind model");
		return false;
	}

	result_.windSpeed = windSpeed;
	result_.windDirection = windDirection;

	size_t landingCount = 0;
	do {
		initializeParameters();

		while (rocket_.pos.z > 0.0 || rocket_.elapsedTime < 0.1) {
			update();

			if (trajectoryMode_ == TrajectoryMode::Parachute) {
				updateParachuteStatus();
			}

			if (rocketType_ == RocketType::Multi) {
				updateDetachedStatus();
			}

			updateParameters();

			calcDynamicForce();

			updateDelta();

			finalUpdate();
		}

		landingCount++;

	} while (landingCount <= rocketAtDetached_.size());

	organizeResult();

	return true;
}

void Solver::initializeParameters() {
	//Second, Third Rocket(Multiple)
	if (rocketAtDetached_.size() != 0 && rocketAtDetached_.size() + 1 > targetRocketIndex_) {

		rocket_ = rocketAtDetached_[targetRocketIndex_ - 1];

		targetRocketIndex_++;

		result_.rocket.push_back({});

		return;
	}

	//First running
	rocketDelta_.mass = specJson_.rocketParam[0].massInitial;
	rocketDelta_.reflLength = specJson_.rocketParam[0].CGLengthInitial;
	rocketDelta_.iyz = specJson_.rocketParam[0].rollingMomentInertiaInitial;
	rocketDelta_.ix = 0.02;
	rocketDelta_.pos = Vector3D(0, 0, 0);
	rocketDelta_.velocity = Vector3D(0, 0, 0);
	rocketDelta_.omega_b = Vector3D(0, 0, 0);
	rocketDelta_.quat = Quaternion(specJson_.env.railElevation, -specJson_.env.railAzimuth + 90);

	rocket_ = rocketDelta_;
}

void Solver::update() {
	air_->update(rocket_.pos.z);

	const double fixedtime = rocketAtDetached_.size() == 0 ? 0.0 : rocketAtDetached_[rocketAtDetached_.size() - 1].elapsedTime;
	combustionTime_ = rocket_.elapsedTime - fixedtime;

	//next target rocket
	if (targetRocketIndex_ == result_.rocket.size()) {
		result_.rocket.push_back({});
	}
}

void Solver::updateParachuteStatus() {
	const bool detectpeakConditon = result_.rocket[targetRocketIndex_].maxHeight > rocket_.pos.z + AppSetting::Setting().simulation.detectPeakThreshold;

	if (detectpeakConditon && !rocket_.detectPeak) {
		rocket_.detectPeak = true;
	}

	if (rocket_.parachuteOpened) {
		return;
	}

	const bool detectpeak = specJson_.rocketParam[targetRocketIndex_].parachute[0].openingType == 0;

	const bool fixedtime = specJson_.rocketParam[targetRocketIndex_].parachute[0].openingType == 1;
	const bool fixedtimeCondition = rocket_.elapsedTime > specJson_.rocketParam[targetRocketIndex_].parachute[0].openingTime;

	const bool time_from_detect_peak = specJson_.rocketParam[targetRocketIndex_].parachute[0].openingType == 2;

	if (detectpeak && detectpeakConditon
		|| fixedtime && fixedtimeCondition) {
		rocket_.parachuteOpened = true;
		result_.rocket[targetRocketIndex_].timeAtParaOpened = rocket_.elapsedTime;
		result_.rocket[targetRocketIndex_].airVelAtParaOpened = rocket_.airSpeed_b.length();
		result_.rocket[targetRocketIndex_].heightAtParaOpened = rocket_.pos.z;
	}

	const bool time_from_detect_peakCondition = 
		rocket_.elapsedTime - result_.rocket[targetRocketIndex_].detectPeakTime > specJson_.rocketParam[targetRocketIndex_].parachute[0].openingTime;

	if (time_from_detect_peak) {
		if (!rocket_.waitForOpenPara && detectpeakConditon) {
			rocket_.waitForOpenPara = true;
		}
		if (rocket_.waitForOpenPara && time_from_detect_peakCondition) {
			rocket_.parachuteOpened = true;
			result_.rocket[targetRocketIndex_].timeAtParaOpened = rocket_.elapsedTime;
			result_.rocket[targetRocketIndex_].airVelAtParaOpened = rocket_.airSpeed_b.length();
			result_.rocket[targetRocketIndex_].heightAtParaOpened = rocket_.pos.z;
			rocket_.waitForOpenPara = false;
		}
	}
}

void Solver::updateDetachedStatus() {
	bool detachCondition = false;

	switch (detachType_)
	{
	case DetachType::BurningFinished:
		detachCondition = specJson_.rocketParam[targetRocketIndex_].engine.isFinishBurning(combustionTime_);
		break;
	case DetachType::Time:
		detachCondition = rocket_.elapsedTime >= detachTime_;
		break;
	case DetachType::SyncPara:
		detachCondition = rocket_.parachuteOpened == true;
		break;
	case DetachType::DoNotDeatch:
		return;
		break;
	}

	if (detachCondition && rocketAtDetached_.size() < 1) {// if need rocket4, 5, 6... , this code should be changed
		//thrust poewr
		double sumThrust = 0;
		for (double t = 0; t <= 0.2; t += dt_) {
			sumThrust += specJson_.rocketParam[targetRocketIndex_].engine.thrustAt(t) * (0.2 - t) / 0.2;
		}

		//next rocket status
		Rocket detach;
		detach.mass = specJson_.rocketParam[targetRocketIndex_ + 2].massInitial;
		detach.reflLength = specJson_.rocketParam[targetRocketIndex_ + 2].CGLengthInitial;
		detach.iyz = specJson_.rocketParam[targetRocketIndex_ + 2].rollingMomentInertiaInitial;
		detach.ix = 0.02;
		detach.pos = rocket_.pos;
		detach.omega_b = Vector3D();
		detach.quat = rocket_.quat;
		detach.velocity = rocket_.velocity;
		rocketAtDetached_.push_back(detach);

		//update this rocket
		rocket_.mass = specJson_.rocketParam[targetRocketIndex_ + 1].massInitial;
		rocket_.reflLength = specJson_.rocketParam[targetRocketIndex_ + 1].CGLengthInitial;
		rocket_.iyz = specJson_.rocketParam[targetRocketIndex_ + 1].rollingMomentInertiaInitial;
		rocket_.velocity -= Vector3D((sumThrust / rocket_.mass) * dt_, 0, 0).applyQuaternion(rocket_.quat);

		//next part
		targetRocketIndex_++;
		result_.rocket.push_back({});
	}
}

void Solver::updateParameters() {
	if ((rocket_.velocity - air_->wind()).length() != 0) {
		rocket_.airSpeed_b = (rocket_.velocity - air_->wind()).applyQuaternion(rocket_.quat.conjugate());
	}
	else {
		rocket_.airSpeed_b = Vector3D();
	}

	const double alpha = atan(rocket_.airSpeed_b.z / (rocket_.airSpeed_b.x + 1e-16));
	const double beta = atan(rocket_.airSpeed_b.y / (rocket_.airSpeed_b.x + 1e-16));
	rocket_.attackAngle = atan(sqrt(rocket_.airSpeed_b.y * rocket_.airSpeed_b.y + rocket_.airSpeed_b.z * rocket_.airSpeed_b.z)
		/ (rocket_.airSpeed_b.x + 1e-16));

	rocket_.Cnp = specJson_.rocketParam[targetRocketIndex_].Cna * alpha;
	rocket_.Cny = specJson_.rocketParam[targetRocketIndex_].Cna * beta;

	rocket_.Cmqp = specJson_.rocketParam[targetRocketIndex_].Cmq;
	rocket_.Cmqy = specJson_.rocketParam[targetRocketIndex_].Cmq;

	if (combustionTime_ <= specJson_.rocketParam[targetRocketIndex_].engine.combustionTime()) {
		rocketDelta_.mass =
			(specJson_.rocketParam[targetRocketIndex_].massFinal - specJson_.rocketParam[targetRocketIndex_].massInitial)
			/ specJson_.rocketParam[targetRocketIndex_].engine.combustionTime();
		rocketDelta_.reflLength =
			(specJson_.rocketParam[targetRocketIndex_].CGLengthFinal - specJson_.rocketParam[targetRocketIndex_].CGLengthInitial)
			/ specJson_.rocketParam[targetRocketIndex_].engine.combustionTime();
		rocketDelta_.iyz = (specJson_.rocketParam[targetRocketIndex_].rollingMomentInertiaFinal - specJson_.rocketParam[targetRocketIndex_].rollingMomentInertiaInitial)
			/ specJson_.rocketParam[targetRocketIndex_].engine.combustionTime();
		rocketDelta_.ix = (0.02 - 0.01) / 3;
	}
	else {
		rocketDelta_.mass = 0;
		rocketDelta_.reflLength = 0;
		rocketDelta_.iyz = 0;
		rocketDelta_.ix = 0;
	}
}

void Solver::calcDynamicForce() {
	force_b_ = Vector3D(0, 0, 0);
	moment_b_ = Vector3D(0, 0, 0);

	//Thrust
	force_b_.x += specJson_.rocketParam[targetRocketIndex_].engine.thrustAt(combustionTime_);

	if (!rocket_.parachuteOpened) {
		//Aero
		const double preForceCalc =
			0.5
			* air_->density()
			* rocket_.airSpeed_b.length()
			* rocket_.airSpeed_b.length()
			* specJson_.rocketParam[targetRocketIndex_].bottomArea;
		force_b_.x -= specJson_.rocketParam[targetRocketIndex_].Cd * preForceCalc * cos(rocket_.attackAngle);
		force_b_.y -= rocket_.Cny * preForceCalc;
		force_b_.z -= rocket_.Cnp * preForceCalc;

		//Moment
		const double preMomentCalc = 
			0.25 
			* air_->density()
			* rocket_.airSpeed_b.length() 
			* specJson_.rocketParam[targetRocketIndex_].length 
			* specJson_.rocketParam[targetRocketIndex_].length
			* specJson_.rocketParam[targetRocketIndex_].bottomArea;
		moment_b_.x = 0;
		moment_b_.y = preMomentCalc * rocket_.Cmqp * rocket_.omega_b.y;
		moment_b_.z = preMomentCalc * rocket_.Cmqy * rocket_.omega_b.z;

		moment_b_.y += force_b_.z * (specJson_.rocketParam[targetRocketIndex_].CPLength - rocket_.reflLength);
		moment_b_.z -= force_b_.y * (specJson_.rocketParam[targetRocketIndex_].CPLength - rocket_.reflLength);

		//Gravity
		force_b_ += Vector3D(0, 0, -air_->gravity()).applyQuaternion(rocket_.quat.conjugate()) * rocket_.mass;
	}


	//update delta
	if (rocket_.pos.length() <= specJson_.env.railLength && rocket_.velocity.z >= 0.0) {//launch
		if (force_b_.x < 0) {
			rocketDelta_.pos = Vector3D();
			rocketDelta_.velocity = Vector3D();
			rocketDelta_.omega_b = Vector3D();
			rocketDelta_.quat = Quaternion();
		}
		else {
			force_b_.y = 0;
			force_b_.z = 0;
			rocketDelta_.pos = rocket_.velocity;

			rocketDelta_.velocity = force_b_.applyQuaternion(rocket_.quat) / rocket_.mass;


			rocketDelta_.omega_b = Vector3D();
			rocketDelta_.quat = Quaternion();
		}
	}
	else if (rocket_.parachuteOpened) {//parachute opened
		const Vector3D paraSpeed = rocket_.velocity;
		const double drag = 0.5 * air_->density() * paraSpeed.z * paraSpeed.z
			* 1.0 * specJson_.rocketParam[targetRocketIndex_].parachute[rocket_.parachuteIndex].Cd;

		rocketDelta_.velocity.z = drag / rocket_.mass - air_->gravity();
		rocketDelta_.velocity.x = 0;
		rocketDelta_.velocity.y = 0;

		rocket_.velocity.x = air_->wind().x;
		rocket_.velocity.y = air_->wind().y;

		rocketDelta_.pos = rocket_.velocity;

		rocketDelta_.omega_b = Vector3D();
		rocketDelta_.quat = Quaternion();
	}
	else if (rocket_.pos.z < -10) {//stop simulation
		rocketDelta_.velocity = Vector3D();
	}
	else {//flight
		if (!launchClear_) {
			result_.launchClearVelocity_ = rocket_.velocity.length();
			launchClear_ = true;
		}

		rocketDelta_.pos = rocket_.velocity;
		rocketDelta_.velocity = force_b_.applyQuaternion(rocket_.quat) / rocket_.mass;

		rocketDelta_.omega_b.x = moment_b_.x / rocket_.ix;
		rocketDelta_.omega_b.y = moment_b_.y / rocket_.iyz;
		rocketDelta_.omega_b.z = moment_b_.z / rocket_.iyz;

		rocketDelta_.quat = rocket_.quat.angularVelocityApplied(rocketDelta_.omega_b);
	}
}

void Solver::updateDelta() {
	rocket_ += rocketDelta_ * dt_;
	rocket_.quat = rocket_.quat.normalized();

	rocket_.elapsedTime += dt_;
}

void Solver::finalUpdate() {
	result_.rocket[targetRocketIndex_].flightData.push_back(rocket_);

	const bool rising = rocket_.velocity.z > 0;

	//height
	if (result_.rocket[targetRocketIndex_].maxHeight < rocket_.pos.z) {
		result_.rocket[targetRocketIndex_].maxHeight = rocket_.pos.z;
		result_.rocket[targetRocketIndex_].detectPeakTime = rocket_.elapsedTime;
	}

	//velocity
	if (result_.rocket[targetRocketIndex_].maxVelocity < rocket_.velocity.length()) {
		result_.rocket[targetRocketIndex_].maxVelocity = rocket_.velocity.length();
	}

	//terminal velocity
	result_.rocket[targetRocketIndex_].terminalVelocity = rocket_.velocity.length();

	//terminal time
	result_.rocket[targetRocketIndex_].terminalTime = rocket_.elapsedTime;

	//attack angle
	const double atkang = launchClear_ && rising ? rocket_.attackAngle * 180 / Constant::PI : 0.0;
	if (result_.rocket[targetRocketIndex_].maxAttackAngle < atkang) {
		result_.rocket[targetRocketIndex_].maxAttackAngle = atkang;
	}

	//normal force
	const double nForce = rising ? sqrt(force_b_.z * force_b_.z + force_b_.y * force_b_.y) : 0.0;
	if (result_.rocket[targetRocketIndex_].maxNormalForce < nForce)
		result_.rocket[targetRocketIndex_].maxNormalForce = nForce;
}

void Solver::organizeResult() {
	for (auto& r : result_.rocket) {
		if (r.flightData[r.flightData.size() - 1].pos.z < 0) {
			r.flightData[r.flightData.size() - 1].pos.z = 0.0;//landing point
		}
		r.lenFromLaunchPoint = r.flightData[r.flightData.size() - 1].pos.length();
		r.latitude = MapFeature::NoshiroSea.latitudeAt(r.flightData[r.flightData.size() - 1].pos.y);
		r.longitude = MapFeature::NoshiroSea.longitudeAt(r.flightData[r.flightData.size() - 1].pos.x);
	}
}