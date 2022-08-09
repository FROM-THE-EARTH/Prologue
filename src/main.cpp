// ------------------------------------------------
// int main()を含む、プログラムの開始関数
// ------------------------------------------------

#include <iostream>
#include <stdexcept>
#include <string>

#include "app/AppSetting.hpp"
#include "app/CommandLine.hpp"
#include "app/Option.hpp"
#include "simulator/SimulatorFactory.hpp"

const auto VERSION = "1.9.1";

void ShowSettingInfo();

int main(int argc, char* argv[]) {
    std::cout << "Prologue v" << VERSION << std::endl << std::endl;

    const auto option = CommandLineOption::ParseArgs(argc, argv);

    ShowSettingInfo();

    // SimulatorBaseインスタンスの生成
    // SimulatorBase抽象クラスのポインタを受け取っているが、実際の中身はDetailSimulator型またはScatterSimulator型
    const auto simulator = SimulatorFactory::Create();

    // インスタンスの生成に失敗したかどうか（simulator == nullptrと同値）
    if (!simulator) {
        CommandLine::PrintInfo(PrintInfoType::Error, "Failed to initialize simulator.");
        return 1;
    }

    // シミュレーション実行
    try {
        if (!simulator->run(option.saveResult && !option.dryRun)) {
            throw std::runtime_error{"Failed to simulate."};
        }
    } catch (const std::exception& e) {
        CommandLine::PrintInfo(PrintInfoType::Error, e.what());
        return 1;
    }

    // Gnuplotで結果をプロット
    if (option.plotResult && !option.dryRun) {
        CommandLine::PrintInfo(PrintInfoType::Information, "Plotting result...");
        simulator->plotToGnuplot();
    }

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
