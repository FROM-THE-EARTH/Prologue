#include <iostream>
#include <string>

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "gnuplot/Gnuplot.hpp"
#include "simulator/Simulator.hpp"

const auto VERSION = "1.8.4";

void ShowSettingInfo();

int main() {
    std::cout << "Prologue v" << VERSION << std::endl << std::endl;

    ShowSettingInfo();

    const auto simulator = Simulator::New(AppSetting::Simulation::dt);

    if (!simulator) {
        CommandLine::PrintInfo(PrintInfoType::Error, "Failed to initialize simulator");
        return 1;
    }

    if (!simulator->run()) {
        CommandLine::PrintInfo(PrintInfoType::Error, "Failed to simulate");
        return 1;
    }

    CommandLine::PrintInfo(PrintInfoType::Information, "Plotting result...");

    simulator->plotToGnuplot();

    Gnuplot::Save();

    CommandLine::Run();

    return 0;
}

void ShowSettingInfo() {
    // Wind model
    const std::string windFile = "Wind data file: " + AppSetting::WindModel::realdataFilename;
    switch (AppSetting::WindModel::type) {
    case WindModelType::Real:
        CommandLine::PrintInfo(PrintInfoType::Information, "Wind model: Real", windFile, "Run detail mode simulation");
        break;

    case WindModelType::Original:
        CommandLine::PrintInfo(PrintInfoType::Information, "Wind model: Original");
        break;

    case WindModelType::OnlyPowerLow:
        CommandLine::PrintInfo(PrintInfoType::Information, "Wind model: Only power low");
        break;

    case WindModelType::NoWind:
        CommandLine::PrintInfo(PrintInfoType::Information, "Wind model: No wind");
        break;
    }
}
