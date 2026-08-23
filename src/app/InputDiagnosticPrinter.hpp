#pragma once

#include <vector>

#include "io/ReadResult.hpp"

namespace InputDiagnosticPrinter {
    void Print(const std::vector<InputIO::Diagnostic>& diagnostics);
}
