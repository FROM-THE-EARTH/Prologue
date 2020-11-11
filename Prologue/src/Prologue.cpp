﻿#include "Simulator.h"

#include "AppSetting.h"
#include "CommandLine.h"

#include <iostream>
#include <string>


const auto VERSION = "Beta";


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
	simulator.run();
}


bool setTitle() {
	FILE* p = _popen(("title Prologue_v" + std::string(VERSION)).c_str(), "w");
	if (p == nullptr) {
		std::cout << "Could not set application title" << std::endl;
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