// ------------------------------------------------
// コマンドラインオプション
// ------------------------------------------------

#pragma once

namespace CommandLineOption {
    struct Option {
        // Whether to save the result
        bool saveResult = true;

        // Whether to plot the result
        bool plotResult = true;

        // false: Neither save nor plot
        bool dryRun = false;
    };

    Option ParseArgs(int argc, char* argv[]);
}
