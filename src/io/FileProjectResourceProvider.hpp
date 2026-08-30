// ------------------------------------------------
// プロジェクトから参照される外部データのファイル読込
// ------------------------------------------------

#pragma once

#include <filesystem>

#include "project/SimulationInputBuilder.hpp"

namespace ProjectIO {
    class FileProjectResourceProvider final : public project::ResourceProvider {
        std::filesystem::path m_projectDirectory;

    public:
        explicit FileProjectResourceProvider(std::filesystem::path projectDirectory);

        std::vector<ThrustData> loadThrustData(
            const std::filesystem::path& projectRelativePath) const override;
        std::vector<AeroCoefficientData> loadAeroCoefficientData(
            const std::filesystem::path& projectRelativePath) const override;
    };
}
