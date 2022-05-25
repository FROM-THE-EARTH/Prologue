#include "CommandLine.hpp"

#include <filesystem>

#include "gnuplot/Gnuplot.hpp"

struct Commands {
    const char* command;
    const char* description;
    void (*operation)();
};

namespace CommandLine {
    namespace Internal {
        size_t Counter = 1;

        std::string OutputDir;

        bool Exit = false;

        void ShowChoices() {}

        void PrintInfoLines() {}

        void ShowHelp();

        void OpenExplorer() {
            const std::filesystem::path path = std::filesystem::current_path().append("result").append(OutputDir);
            const std::string comamnd        = "explorer " + path.string();
            if (system(comamnd.c_str()) != 0) {
                PrintInfo(PrintInfoType::Error, "Could not open the folder.", path.string().c_str());
            }
        }

        void ExitApplication() {
            Exit = true;
        }
    }

    const Commands commands[] = {{"show", "Show result in Gnuplot", Gnuplot::Show},
                                 {"open", "Open result folder in explorer", Internal::OpenExplorer},
                                 {"help", "Show commands", Internal::ShowHelp},
                                 {"exit", "Exit application", Internal::ExitApplication}};

    namespace Internal {
        void ShowHelp() {
            for (auto& c : commands) {
                std::cout << "\t" << c.command << ": " << c.description << std::endl;
            }
        }
    }

    void Run() {
        std::cout << "Input the command" << std::endl;
        Internal::ShowHelp();

        std::string cmd;
        bool unknown = true;

        while (!Internal::Exit) {
            unknown = true;

            std::cout << ">";
            std::cin >> cmd;
            for (auto& c : commands) {
                if (cmd == c.command) {
                    unknown = false;
                    c.operation();
                }
            }

            if (unknown) {
                Internal::ShowHelp();
            }
        }
    }

    void SetOutputDir(const std::string dirname) {
        Internal::OutputDir = dirname;
    }
}
