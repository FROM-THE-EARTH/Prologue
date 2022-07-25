#include "CommandLine.hpp"

#include <filesystem>
#include <functional>

#include "gnuplot/Gnuplot.hpp"

struct Command {
    const char* input;
    const char* description;
    std::function<void()> handler;
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
#if defined(_WIN32) || defined(WIN32)
            const std::filesystem::path path = std::filesystem::current_path().append("result").append(OutputDir);
            const std::string comamnd        = "explorer " + path.string();
            system(comamnd.c_str());
#else
            PrintInfo(PrintInfoType::Error, "This command is only available on windows");
#endif
        }

        void ExitApplication() {
            Exit = true;
        }
    }

    const Command commands[] = {{"show", "Show result in Gnuplot", Gnuplot::Show},
                                {"open", "Open result folder in explorer", Internal::OpenExplorer},
                                {"help", "Show commands", Internal::ShowHelp},
                                {"exit", "Exit application", Internal::ExitApplication}};

    namespace Internal {
        void ShowHelp() {
            for (const auto& command : commands) {
                std::cout << "\t" << command.input << ": " << command.description << std::endl;
            }
        }
    }

    void Run() {
        std::cout << "Input the command" << std::endl;
        Internal::ShowHelp();

        std::string input;
        bool unknown = true;

        while (!Internal::Exit) {
            unknown = true;

            std::cout << ">";
            std::cin >> input;
            for (const auto& command : commands) {
                if (input == command.input) {
                    unknown = false;
                    command.handler();
                }
            }

            if (unknown) {
                Internal::ShowHelp();
            }
        }
    }

    void SetOutputDir(const std::string& dirname) {
        Internal::OutputDir = dirname;
    }
}
