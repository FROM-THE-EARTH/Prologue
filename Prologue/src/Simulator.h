#pragma once
#include "Solver.h"
#include "SpecJson.h"
#include "Gnuplot.h"

#include <string>
#include <vector>


enum class SimulationMode : int {
	Scatter = 1,
	Detail
};


class Simulator {

	//settings
	std::string jsonFilename_;
	SimulationMode simulationMode_;
	RocketType rocketType_;
	TrajectoryMode trajectoryMode_;
	DetachType detachType_;
	double detachTime_;

	//simulate
	const double dt_;
	double windSpeed_ = 0.0;
	double windDirection_ = 0.0;//direction is clockwise from the north
	bool solved_ = false;

	//from json
	SpecJson specJson_;

	//result
	std::string outputDirName_;
	SolvedResult detailResult_;
	std::vector<SolvedResult> scatterResult_;

public:

	Simulator(double dt)
		:dt_(dt)
	{}

	bool run();

	void plotToGnuplot() {
		switch (simulationMode_)
		{
		case SimulationMode::Scatter:
			Gnuplot::Plot(scatterResult_);
			break;

		case SimulationMode::Detail:
			Gnuplot::Plot(detailResult_);
			break;
		}
	}

private:

	bool initialize();

	void setJSONFile();
	void setSimulationMode();
	void setTrajectoryMode();
	void setWindCondition();
	void setDetachType();
	void setDetachTime();

	void scatterSimulation();
	void detailSimulation();

	void singleThreadSimulation();
	void multiThreadSimulation();

	void solverRunner(double windSpeed, double windDir, SolvedResult* result, bool* finish, bool* error);

	SolvedResult formatResultForScatter(const SolvedResult& result);

	void eraseNotLandingPoint(SolvedResult* result);

	void createResultDirectory();

	void saveResult();

	bool updateWindCondition();
};