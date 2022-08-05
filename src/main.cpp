// ------------------------------------------------
// int main()を含む、プログラムの開始関数
// ------------------------------------------------

#include <iostream>
#include <string>

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "simulator/Simulator.hpp"

const auto VERSION = "1.8.10";

// Comamnd line option
struct Option {
    // Whether to save the result
    bool saveResult = true;

    // Whether to plot the result
    bool plotResult = true;

    // false: Neither save nor plot
    bool dryRun = false;
};

Option GetOption(int argc, char* argv[]);

void ShowSettingInfo();

int main(int argc, char* argv[]) {
    std::cout << "Prologue v" << VERSION << std::endl << std::endl;

    const auto option = GetOption(argc, argv);

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
    if (!simulator->run(option.saveResult && !option.dryRun)) {
        CommandLine::PrintInfo(PrintInfoType::Error, "Failed to simulate");
        return 1;
    }

    // Gnuplotで結果をプロット
    if (option.plotResult && !option.dryRun) {
        CommandLine::PrintInfo(PrintInfoType::Information, "Plotting result...");
        simulator->plotToGnuplot();
    }

    return 0;
}

Option GetOption(int argc, char* argv[]) {
    Option option;

    for (int i = 0; i < argc; i++) {
        const std::string opt = argv[i];

        if (opt == "--no-save") {
            option.saveResult = false;
        } else if (opt == "--no-plot") {
            option.plotResult = false;
        } else if (opt == "--dry-run") {
            option.dryRun = true;
        }
    }

    return option;
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
