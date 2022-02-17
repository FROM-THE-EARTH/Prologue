#include <iostream>
#include <string>

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "gnuplot/Gnuplot.hpp"
#include "solver/Simulator.hpp"

#if defined(_WIN32) || defined(WIN32)
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

const auto VERSION = "1.3.1";

bool setTitle();
void showSettingInfo();

int main() {
    if (!setTitle()) {
        return 0;
    }

    if (!AppSetting::Initialize()) {
        std::cout << "Failed to initialize application" << std::endl;
        return 0;
    }

    showSettingInfo();

    Simulator simulator(AppSetting::GetSetting().simulation.dt);
    if (!simulator.run()) {
        return 0;
    }

    simulator.plotToGnuplot();

    Gnuplot::Save();

    CommandLine::Run();
}

bool setTitle() {
    FILE* p = POPEN(("title Prologue_v" + std::string(VERSION)).c_str(), "w");
    if (p == nullptr) {
        CommandLine::PrintInfo(PrintInfoType::Error, "Could not set application title");
        return false;
    } else {
        PCLOSE(p);
        return true;
    }
}

void showSettingInfo() {
    // Wind model
    const std::string s = "Wind data file: " + AppSetting::GetSetting().windModel.realdataFilename;
    switch (AppSetting::GetSetting().windModel.type) {
    case WindModelType::Real:
        CommandLine::PrintInfo(PrintInfoType::Information, "Wind model: Real", s.c_str(), "Run detail mode simulation");
        break;

    case WindModelType::Original:
        CommandLine::PrintInfo(PrintInfoType::Information, "Wind model: Original");
        break;

    case WindModelType::OnlyPowerLow:
        CommandLine::PrintInfo(PrintInfoType::Information, "Wind model: Only power low");
        break;
    }
}
