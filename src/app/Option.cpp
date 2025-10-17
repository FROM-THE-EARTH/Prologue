// ------------------------------------------------
// Option.hppの実装
// ------------------------------------------------

#include "Option.hpp"

#include <string>

#include "app/CommandLine.hpp"

namespace CommandLineOption {
    Option ParseArgs(int argc, char* argv[]) {
        Option option;

        for (int i = 0; i < argc; i++) {
            const std::string arg = argv[i];

            if (arg.starts_with("--")) {
                const auto opt = arg.substr(2);
                if (opt == "no-save") {
                    option.saveResult = false;
                } else if (opt == "no-plot") {
                    option.plotResult = false;
                } else if (opt == "dry-run") {
                    option.dryRun = true;
                } else if (opt == "open-result") {
                    option.openResultFolder = true;
                } else if (opt == "spec-file") {
                    option.specifySpecFile = true;
                    if (i + 1 < argc) {
                        option.specFilePath = argv[i + 1];
                        i++; // Skip the next argument as it is the spec file path
                    } else {
                        CommandLine::PrintInfo(PrintInfoType::Error, "No spec file path provided after --spec-file.");
                        option.specifySpecFile = false; // Reset to false if no path is provided
                    }
                } else {
                    CommandLine::PrintInfo(PrintInfoType::Warning,
                                           "Specified option \"" + opt + "\" is not available.");
                }
            }
        }

        return option;
    }
}
