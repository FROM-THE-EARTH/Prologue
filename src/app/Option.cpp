// ------------------------------------------------
// Option.hppの実装
// ------------------------------------------------

#include "Option.hpp"

#include <iostream>
#include <string>

namespace CommandLineOption {
    Option ParseArgs(int argc, char* argv[]) {
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
}
