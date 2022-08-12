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
                } else {
                    CommandLine::PrintInfo(PrintInfoType::Warning,
                                           "Specified option \"" + opt + "\" is not available.");
                }
            }
        }

        return option;
    }
}
