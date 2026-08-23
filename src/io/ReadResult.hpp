#pragma once

#include <string>
#include <utility>
#include <vector>

namespace InputIO {
    enum class DiagnosticLevel {
        Information,
        Warning,
        Error,
    };

    struct Diagnostic {
        DiagnosticLevel level;
        std::vector<std::string> lines;
    };

    template <typename T>
    struct ReadResult {
        T value;
        std::vector<Diagnostic> diagnostics;
    };
}
