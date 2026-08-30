// ------------------------------------------------
// CLIの結果出力ディレクトリの命名と作成
// ------------------------------------------------

#pragma once

#include <filesystem>
#include <string>

#include "core/SimulationInput.hpp"
#include "runner/SimulationRunSettings.hpp"

namespace cli::ResultDirectory {
    std::string BuildName(const std::string& specificationName,
                          SimulationMode mode,
                          const SimulationInput& input,
                          const std::string& measuredWindFilename);

    std::filesystem::path Create(const std::string& directoryName);
}
