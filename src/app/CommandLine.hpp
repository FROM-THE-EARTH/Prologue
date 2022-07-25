#pragma once

#include <iostream>
#include <optional>
#include <string>

enum class PrintInfoType { Information, Warning, Error };

namespace CommandLine {
    namespace Internal {
        extern size_t Counter;

        void ShowChoices();

        void PrintInfoLines();

        template <typename T, class... Args>
        void ShowChoices(const T& s, Args... choices) {
            std::cout << Counter << ": " << s << std::endl;

            Counter++;

            ShowChoices(std::forward<Args>(choices)...);
        }

        template <typename T, class... Args>
        void PrintInfoLines(const T& line, Args... lines) {
            std::cout << "   " << line << std::endl;

            PrintInfoLines(std::forward<Args>(lines)...);
        }

        template <typename T, class... Args>
        void PrintInfoFirstLine(PrintInfoType type, const T& line, Args... lines) {
            switch (type) {
            case PrintInfoType::Information:
                std::cout << "I: ";
                break;

            case PrintInfoType::Warning:
                std::cout << "W: ";
                break;

            case PrintInfoType::Error:
                std::cout << "E: ";
                break;
            }

            std::cout << line << std::endl;

            PrintInfoLines(std::forward<Args>(lines)...);
        }
    }

    template <typename T, class... Args>
    void Question(const T& question, Args... choices) {
        std::cout << "<!===" << question << "===!>" << std::endl;

        Internal::Counter = 1;

        Internal::ShowChoices(std::forward<Args>(choices)...);
    }

    template <typename Type>
    Type InputIndex(size_t size) {
        int var = -1;

        std::cin >> var;

        while (var <= 0 || var > static_cast<int>(size)) {
            std::cin >> var;
        }

        std::cout << std::endl;

        return static_cast<Type>(var);
    }

    template <class... Args>
    void PrintInfo(PrintInfoType type, Args... lines) {
        Internal::PrintInfoFirstLine(type, std::forward<Args>(lines)...);
        std::cout << std::endl;
    }

    void Run();

    void SetOutputDir(const std::string& dirname);
}
