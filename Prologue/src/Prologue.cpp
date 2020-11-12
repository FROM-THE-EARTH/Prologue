#include "Simulator.h"

#include "AppSetting.h"
#include "CommandLine.h"
#include "Gnuplot.h"

#include <iostream>
#include <string>


const auto VERSION = "1.0.0";


bool setTitle();
void showSettingInfo();


int main()
{
	if (!setTitle()) {
		return 1;
	}

	if (!AppSetting::Initialize()) {
		std::cout << "Failed to initialize application" << std::endl;
		return 1;
	}

	showSettingInfo();

	Simulator simulator;
	if (!simulator.run()) {
		return 0;
	}

	simulator.plotToGnuplot();

	Gnuplot::Save();

	CommandLine::Run();
}


bool setTitle() {
	FILE* p = _popen(("title Prologue_v" + std::string(VERSION)).c_str(), "w");
	if (p == nullptr) {
		CommandLine::PrintInfo(PrintInfoType::Error, "Could not set application title");
		return false;
	}
	else {
		_pclose(p);
		return true;
	}
}


void showSettingInfo() {
	//Wind model
	const std::string s = "Wind data file: " + AppSetting::Setting().windModel.realdataFilename;
	switch (AppSetting::Setting().windModel.type)
	{
	case WindModelType::Real:
		CommandLine::PrintInfo(PrintInfoType::Information,
			"Wind model: Real",
			s.c_str(),
			"Run detail mode simulation");
		break;

	case WindModelType::Original:
		CommandLine::PrintInfo(PrintInfoType::Information,
			"Wind model: Original");
		break;

	case WindModelType::OnlyPowerLow:
		CommandLine::PrintInfo(PrintInfoType::Information,
			"Wind model: Only power low");
		break;
	}
}