#include <iostream>
#include <string>

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "gnuplot/Gnuplot.hpp"
#include "solver/Simulator.hpp"

const auto VERSION = "1.4.0";

void showSettingInfo();

int main() {
    std::cout << "Prologue v" << VERSION << std::endl << std::endl;

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
