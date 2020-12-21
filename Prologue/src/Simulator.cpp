#include "Simulator.h"

#include "CommandLine.h"
#include "SpecJsonReader.h"
#include "ResultSaver.h"
#include "AppSetting.h"
#include "MapFeature.h"

#include <iostream>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>


bool Simulator::run() {
	if (!initialize()) {
		CommandLine::PrintInfo(PrintInfoType::Error, "Could not initialize Simulator");
		return false;
	}

	createResultDirectory();

	Gnuplot::Initialize(outputDirName_.c_str(), MapFeature::GetMapFromName(specJson_.env.place).map);

	const auto start = std::chrono::system_clock::now();

	switch (simulationMode_)
	{
	case SimulationMode::Scatter:
		scatterSimulation();
		break;

	case SimulationMode::Detail:
		detailSimulation();
		break;
	}

	if (!solved_) {
		CommandLine::PrintInfo(PrintInfoType::Error, "Failed to simulate");
		return false;
	}

	const auto end = std::chrono::system_clock::now();
	const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	const std::string s = "Finish processing: " + std::to_string(elapsed / 1000000.0) + "[s]";
	CommandLine::PrintInfo(PrintInfoType::Information, s.c_str());

	saveResult();

	const std::string str = "Result is saved in \"" + outputDirName_ + "/\"";
	CommandLine::PrintInfo(PrintInfoType::Information, str.c_str());

	if (simulationMode_ == SimulationMode::Scatter) {
		for (auto& r : scatterResult_) {
			eraseNotLandingPoint(&r);
		}
	}

	CommandLine::SetOutputDir(outputDirName_);

	return true;
}


bool Simulator::initialize() {
	//settings on commandline
	setJSONFile();

	if (AppSetting::Setting().windModel.type == WindModelType::Real) {
		simulationMode_ = SimulationMode::Detail;
	}
	else {
		setSimulationMode();
	}

	setTrajectoryMode();

	if (AppSetting::Setting().windModel.type != WindModelType::Real
		&& simulationMode_ == SimulationMode::Detail) {
		setWindCondition();
	}

	if (SpecJsonReader::IsMultipleRocket(jsonFilename_)) {
		CommandLine::PrintInfo(PrintInfoType::Information, "This is Multiple Rocket");
		rocketType_ = RocketType::Multi;
		setDetachType();
		if (detachType_ == DetachType::Time) {
			setDetachTime();
		}
	}
	else {
		rocketType_ = RocketType::Single;
	}

	std::cout << "----------------------------------------------------------" << std::endl;

	//read json
	specJson_ = SpecJsonReader::ReadJson(jsonFilename_);

	//output
	outputDirName_ = jsonFilename_;
	outputDirName_.erase(outputDirName_.size() - 5, 5);
	outputDirName_ += "[";

	const std::filesystem::path f = AppSetting::Setting().windModel.realdataFilename;
	switch (AppSetting::Setting().windModel.type)
	{
	case WindModelType::Real:
		outputDirName_ += "(" + f.stem().string() + ")";
		break;
	case WindModelType::Original:
		outputDirName_ += "original";
		break;
	case WindModelType::OnlyPowerLow:
		outputDirName_ += "powerlow";
		break;
	}

	switch (simulationMode_)
	{
	case SimulationMode::Scatter:
		outputDirName_ += "_scatter";
		break;
	case SimulationMode::Detail:
		outputDirName_ += "_detail";
		break;
	}

	switch (trajectoryMode_)
	{
	case TrajectoryMode::Parachute:
		outputDirName_ += "_para";
		break;
	case TrajectoryMode::Trajectory:
		outputDirName_ += "_trajectory";
		break;
	}

	outputDirName_ += "]";

	return true;
}


void Simulator::setJSONFile() {
	std::cout << "<!===Set JSON File===!>" << std::endl;
	std::vector<std::string> existJSONs;
	for (const std::filesystem::directory_entry& x : std::filesystem::directory_iterator("input/json")) {
		existJSONs.push_back(x.path().filename().string());
	}
	for (size_t i = 0; i < existJSONs.size(); i++) {
		std::cout << i + 1 << ": " << existJSONs[i] << std::endl;
	}
	const size_t jsonIndex = CommandLine::InputIndex<size_t>(existJSONs.size());
	jsonFilename_ = existJSONs[jsonIndex - 1];
}


void Simulator::setSimulationMode() {
	CommandLine::Question("Set Simulation Mode", "Scatter Mode", "Detail Mode");
	simulationMode_ = CommandLine::InputIndex<SimulationMode>(2);
}


void Simulator::setTrajectoryMode() {
	CommandLine::Question("Set Falling Type", "Trajectory", "Parachute");
	trajectoryMode_ = CommandLine::InputIndex<TrajectoryMode>(2);
}


void Simulator::setWindCondition() {
	std::cout << "Input Wind Velocity[m/s]" << std::endl;
	std::cin >> windSpeed_;
	std::cout << std::endl;

	std::cout << "Input Wind Direction[deg] (North: 0, East: 90)" << std::endl;
	std::cin >> windDirection_;
	std::cout << std::endl;
}


void Simulator::setDetachType() {
	CommandLine::Question("Set Detach Type", "When burning finished", "Specify time", "Concurrently with parachute", "Do not detach");
	detachType_ = CommandLine::InputIndex<DetachType>(4);
}


void Simulator::setDetachTime() {
	CommandLine::Question("Set Detach Time");
	std::cin >> detachTime_;
	std::cout << std::endl;
}


void Simulator::scatterSimulation() {
	windSpeed_ = AppSetting::Setting().simulation.windSpeedMin;
	windDirection_ = 0.0;

	solved_ = true;

	if (AppSetting::Setting().processing.multiThread) {
		multiThreadSimulation();
	}
	else {
		singleThreadSimulation();
	}
}

void Simulator::detailSimulation() {
	Solver solver(dt_, rocketType_, trajectoryMode_, detachType_,
		detachTime_, specJson_);

	solved_ = solver.run(windSpeed_, windDirection_);
	if (!solved_) {
		return;
	}

	detailResult_ = solver.getResult();
}

void Simulator::singleThreadSimulation() {
	while (1) {
		Solver solver(dt_, rocketType_, trajectoryMode_, detachType_,
			detachTime_, specJson_);

		solved_ &= solver.run(windSpeed_, windDirection_);
		if (!solved_) {
			break;
		}

		auto result = solver.getResult();

		result = formatResultForScatter(result);
		scatterResult_.push_back(result);

		if (!updateWindCondition()) {
			break;
		}
	}
}

void Simulator::multiThreadSimulation() {
	const size_t threadCount = 2;
	bool finish = false;
	size_t simulated = 1;

	while (!finish) {
		bool e = false;

		SolvedResult results[threadCount];
		std::thread threads[threadCount];
		bool finished[threadCount] = { false };
		bool error[threadCount] = { false };

		simulated = 1;

		threads[0] = std::thread(&Simulator::solverRunner, this, windSpeed_, windDirection_, &results[0], &finished[0], &error[0]);
		threads[0].detach();
		for (size_t i = 1; i < threadCount; i++) {
			finish = !updateWindCondition();
			if (finish) {
				break;
			}

			threads[i] = std::thread(&Simulator::solverRunner, this, windSpeed_, windDirection_, &results[i], &finished[i], &error[i]);
			threads[i].detach();
			simulated++;
		}

		//wait threads
		while (1) {
			bool f = true;
			for (size_t i = 0; i < simulated; i++) {
				f = f && (finished[i] || error[i]);
				e = e || error[i];
			}
			if (f) {
				break;
			}
			std::cout << "";
		}

		//error occured
		if (e) {
			solved_ = false;
			break;
		}

		for (size_t i = 0; i < simulated; i++) {
			scatterResult_.push_back(results[i]);
		}

		finish = !updateWindCondition();
	}
}

void Simulator::solverRunner(double windSpeed, double windDir, SolvedResult* result, bool* finish, bool* error) {
	Solver solver(dt_, rocketType_, trajectoryMode_, detachType_,
		detachTime_, specJson_);
	
	if (*error = !solver.run(windSpeed, windDir); *error) {
		return;
	}

	*result = solver.getResult();
	*result = formatResultForScatter(*result);
	*finish = true;
}

SolvedResult Simulator::formatResultForScatter(const SolvedResult& result) {
	SolvedResult res = result;

	for (auto& r : res.rocket) {
		Rocket landing = r.flightData[r.flightData.size() - 1];
		const auto v = std::vector<Rocket>(1, landing);
		r.flightData = v;
	}

	return res;
}

void Simulator::eraseNotLandingPoint(SolvedResult* result) {
	const int size = static_cast<int>(result->rocket.size());
	for (int i = size - 1; i >= 0; i--) {
		if (result->rocket[i].flightData[0].pos.z != 0.0) {
			result->rocket.erase(result->rocket.begin() + i);
		}
	}
}

void Simulator::createResultDirectory() {
	const std::filesystem::path result = "result";
	if (!std::filesystem::exists(result)) {
		if (!std::filesystem::create_directory(result)) {
			CommandLine::PrintInfo(PrintInfoType::Error, "Failed to create result directory");
		}
	}

	const std::filesystem::path output = "result/" + outputDirName_;
	if (!std::filesystem::exists(output)) {
		if (!std::filesystem::create_directory(output)) {
			CommandLine::PrintInfo(PrintInfoType::Error, "Failed to create result directory");
		}
	}
}

void Simulator::saveResult() {
	const size_t rocketNum = simulationMode_ == SimulationMode::Scatter ?
		scatterResult_[0].rocket.size() : detailResult_.rocket.size();

	for (size_t i = 0; i < rocketNum; i++) {
		const std::string s = "result/" + outputDirName_ + "/result_rocket" + std::to_string(i + 1) + ".csv";

		switch (simulationMode_)
		{
		case SimulationMode::Scatter:
			ResultSaver::OpenFile(s.c_str(), scatterResult_.size());

			for (auto& result : scatterResult_) {
				ResultSaver::WriteLine(result, i);
			}
			break;

		case SimulationMode::Detail:
			ResultSaver::OpenFile(s.c_str(), 1);

			ResultSaver::WriteLine(detailResult_, i);

			break;
		}

		ResultSaver::Close();
	}
}

bool Simulator::updateWindCondition() {
	windDirection_ += AppSetting::Setting().simulation.windDirInterval;
	if (windDirection_ >= 360.0) {
		windDirection_ = 0.0;
		windSpeed_ += 1.0;
		if (windSpeed_ > AppSetting::Setting().simulation.windSpeedMax) {
			return false;
		}
	}

	return true;
}