#include "cli/InputDiagnosticPrinter.hpp"

#include <iostream>

namespace InputDiagnosticPrinter {
    void Print(const std::vector<InputIO::Diagnostic>& diagnostics) {
        for (const auto& diagnostic : diagnostics) {
            if (diagnostic.lines.empty()) continue;

            switch (diagnostic.level) {
            case InputIO::DiagnosticLevel::Information:
                std::cout << "I: ";
                break;
            case InputIO::DiagnosticLevel::Warning:
                std::cout << "W: ";
                break;
            case InputIO::DiagnosticLevel::Error:
                std::cout << "E: ";
                break;
            }
            std::cout << diagnostic.lines.front() << std::endl;
            for (size_t i = 1; i < diagnostic.lines.size(); i++) {
                std::cout << "   " << diagnostic.lines[i] << std::endl;
            }
            std::cout << std::endl;
        }
    }
}
