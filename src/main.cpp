// ------------------------------------------------
// int main()を含む、プログラムの開始関数
// ------------------------------------------------

#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "app/CommandLine.hpp"
#include "app/FetchVersion.hpp"
#include "app/InputDiagnosticPrinter.hpp"
#include "app/Option.hpp"
#include "config/ApplicationSettings.hpp"
#include "misc/Platform.hpp"
#include "io/ApplicationSettingsReader.hpp"
#include "simulator/SimulatorFactory.hpp"

const auto VERSION = "1.11.1";

void ShowSettingInfo(const ApplicationSettings& settings);

int main(int argc, char* argv[]) {
    auto settingsReadResult = ApplicationSettingsReader::Read("prologue.settings.json");
    InputDiagnosticPrinter::Print(settingsReadResult.diagnostics);
    const auto applicationSettings = std::move(settingsReadResult.value);

    std::cout << "Prologue v" << VERSION << std::endl;

    const auto latest_version = FetchVersion::GetLatestVersionString();
	if (latest_version != "N/A" && latest_version != VERSION) {
		CommandLine::PrintInfo(PrintInfoType::Information,
			"A newer version of Prologue is available: v" + latest_version + "."
		);
		CommandLine::PrintInfo(PrintInfoType::Information,
			"Please visit https://github.com/FROM-THE-EARTH/Prologue/releases/latest for more information."
		);
	}


    const auto option = CommandLineOption::ParseArgs(argc, argv);

    ShowSettingInfo(applicationSettings);

    // SimulatorBaseインスタンスの生成
    // SimulatorBase抽象クラスのポインタを受け取っているが、実際の中身はDetailSimulator型またはScatterSimulator型
    const auto simulator = SimulatorFactory::Create(option, applicationSettings);

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

    // 結果フォルダを開く
    if (option.openResultFolder) {
        const auto path = std::filesystem::current_path() / "result" / simulator->getOutputDirectory();
#if PLATFORM_WINDOWS
        system(("explorer " + path.string()).c_str());
#elif PLATFORM_MACOS
        system(("open " + path.string()).c_str());
#else
        system(("nautilus -w \"" + path.string() + "\" &").c_str());
#endif
    }

    return 0;
}

void ShowSettingInfo(const ApplicationSettings& settings) {
    // Wind model
    const std::string windFile = "Wind data file: " + settings.measuredWindFilename;
    switch (settings.solver.wind.type) {
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
