// ------------------------------------------------
// コマンドラインオプション
// ------------------------------------------------

#pragma once

#include <string>

namespace CommandLineOption {
    struct Option {
        // Whether to save the result
        bool saveResult = true;

        // Whether to plot the result
        bool plotResult = true;

        // false: Neither save nor plot
        bool dryRun = false;

        // Whether to open result folder automatically
        bool openResultFolder = false;

        // Whether to specify a spec file
        bool specifySpecFile = false;

        // The path to the spec file (valid only if specifySpecFile is true)
        std::string specFilePath;
    };

    Option ParseArgs(int argc, char* argv[]);
}
