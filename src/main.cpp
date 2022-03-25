#include <iostream>
#include <string>

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "gnuplot/Gnuplot.hpp"
#include "solver/Simulator.hpp"

const auto VERSION = "1.4.2";

void ShowSettingInfo();

int main() {
    std::cout << "Prologue v" << VERSION << std::endl << std::endl;

    ShowSettingInfo();

    Simulator simulator(AppSetting::Simulation::dt);
    if (!simulator.run()) {
        return 1;
    }

    simulator.plotToGnuplot();

    Gnuplot::Save();

    CommandLine::Run();

    return 0;
}

void ShowSettingInfo() {
    // Wind model
    const std::string s = "Wind data file: " + AppSetting::WindModel::realdataFilename;
    switch (AppSetting::WindModel::type) {
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
