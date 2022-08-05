// ------------------------------------------------
// int main()を含む、プログラムの開始関数
// ------------------------------------------------

#include <iostream>
#include <string>

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "simulator/Simulator.hpp"

const auto VERSION = "1.8.10";

void ShowSettingInfo();

int main() {
    std::cout << "Prologue v" << VERSION << std::endl << std::endl;

    ShowSettingInfo();

    // Simulatorインスタンスの生成
    // Simulator抽象クラスのポインタを受け取っているが、実際の中身はDetailSimulator型またはScatterSimulator型
    const auto simulator = Simulator::New(AppSetting::Simulation::dt);

    // インスタンスの生成に失敗したかどうか（simulator == nullptrと同値）
    if (!simulator) {
        CommandLine::PrintInfo(PrintInfoType::Error, "Failed to initialize simulator");
        return 1;
    }

    // シミュレーション実行
    if (!simulator->run()) {
        CommandLine::PrintInfo(PrintInfoType::Error, "Failed to simulate");
        return 1;
    }

    // Gnuplotで結果をプロット
    CommandLine::PrintInfo(PrintInfoType::Information, "Plotting result...");

    simulator->plotToGnuplot();

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
