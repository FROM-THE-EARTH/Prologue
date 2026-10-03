// ------------------------------------------------
// int main()を含む、プログラムの開始関数
// ------------------------------------------------

#include <chrono>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "cli/CommandLine.hpp"
#include "cli/FetchVersion.hpp"
#include "cli/InputDiagnosticPrinter.hpp"
#include "cli/Option.hpp"
#include "cli/ProgressObserver.hpp"
#include "cli/ResultDirectory.hpp"
#include "cli/ResultSaveProgressObserver.hpp"
#include "cli/SimulationRequest.hpp"
#include "config/ApplicationSettings.hpp"
#include "io/ApplicationSettingsReader.hpp"
#include "misc/Platform.hpp"
#include "result/ResultSaver.hpp"
#include "runner/SimulationRunner.hpp"

const auto VERSION = "1.11.1";

void ShowSettingInfo(const ApplicationSettings& settings);

int main(int argc, char* argv[]) {
    try {
		// Import application settings from prologue.settings.json
		auto settingsReadResult = ApplicationSettingsReader::Read("prologue.settings.json");
		InputDiagnosticPrinter::Print(settingsReadResult.diagnostics);
		const auto applicationSettings = std::move(settingsReadResult.value);

		// Fetch the latest version of Prologue from GitHub and compare it with the current version
		std::cout << "Prologue v" << VERSION << std::endl;
		const auto latestVersion = FetchVersion::GetLatestVersionString();
		if (latestVersion != "N/A" && latestVersion != VERSION) {
			CommandLine::PrintInfo(PrintInfoType::Information,
								"A newer version of Prologue is available: v" + latestVersion + ".");
			CommandLine::PrintInfo(
				PrintInfoType::Information,
				"Please visit https://github.com/FROM-THE-EARTH/Prologue/releases/latest for more information.");
		}

		// Parse command line arguments and prepare the simulation request
        const auto option = CommandLineOption::ParseArgs(argc, argv);
        ShowSettingInfo(applicationSettings);

		// Construct the simulation input
        const auto prepared = cli::PrepareSimulation(option, applicationSettings);
        const auto outputDirectoryName = cli::ResultDirectory::BuildName(
            prepared.specificationName,
            prepared.mode,
            prepared.input,
            applicationSettings.measuredWindFilename);

		// Run the simulation
        using RunOutput = std::variant<SimulationResult, std::vector<SimulationResult>>;
        const auto start = std::chrono::system_clock::now();
        RunOutput output;
        if (prepared.mode == SimulationMode::Detail) {
            output = SimulationRunner::RunDetail(prepared.input);
        } else {
            cli::ProgressObserver progress;
            output = SimulationRunner::RunScatter(prepared.input,
                                                  applicationSettings.scatter,
                                                  applicationSettings.execution,
                                                  &progress);
        }
        const auto end = std::chrono::system_clock::now();
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        CommandLine::PrintInfo(PrintInfoType::Information,
                               "Finish processing: " + std::to_string(elapsed / 1000000.0) + "[s]");

		// Post-processing (save results and open result folder if specified)
        const bool saveResult = option.saveResult && !option.dryRun;
        std::optional<std::filesystem::path> outputDirectory;
        if (saveResult || option.openResultFolder) {
            outputDirectory = cli::ResultDirectory::Create(outputDirectoryName);
        }

        if (saveResult) {
            CommandLine::PrintInfo(PrintInfoType::Information, "Saving result...");
            const std::string directoryPrefix = outputDirectory->string() + "/";
            if (prepared.mode == SimulationMode::Detail) {
                cli::ResultSaveProgressObserver progress;
                ResultSaver::SaveDetail(directoryPrefix,
                                        std::get<SimulationResult>(output),
                                        prepared.input.environment,
                                        applicationSettings.result.precision,
                                        &progress);
            } else {
                ResultSaver::SaveScatter(directoryPrefix,
                                         std::get<std::vector<SimulationResult>>(output),
                                         prepared.input.environment,
                                         applicationSettings.result.precision);
            }
            CommandLine::PrintInfo(PrintInfoType::Information,
                                   "Result is saved in \"" + outputDirectoryName + "/\"");
        }

        if (option.openResultFolder) {
            const auto path = std::filesystem::current_path() / *outputDirectory;
#if PLATFORM_WINDOWS
            system(("explorer " + path.string()).c_str());
#elif PLATFORM_MACOS
            system(("open " + path.string()).c_str());
#else
            system(("nautilus -w \"" + path.string() + "\" &").c_str());
#endif
        }
    } catch (const std::exception& e) {
        CommandLine::PrintInfo(PrintInfoType::Error, e.what());
        return 1;
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
