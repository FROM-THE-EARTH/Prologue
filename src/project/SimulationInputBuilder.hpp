// ------------------------------------------------
// 保存用プロジェクト諸元からcore実行入力への変換
// ------------------------------------------------

#pragma once

#include <filesystem>
#include <vector>

#include "core/SimulationInput.hpp"
#include "project/ProjectDocument.hpp"
#include "rocket/AeroCoefficient.hpp"
#include "rocket/Engine.hpp"

namespace project {
    class ResourceProvider {
    public:
        virtual ~ResourceProvider() = default;

        virtual std::vector<ThrustData> loadThrustData(
            const std::filesystem::path& projectRelativePath) const = 0;
        virtual std::vector<AeroCoefficientData> loadAeroCoefficientData(
            const std::filesystem::path& projectRelativePath) const = 0;
    };

    SimulationInput BuildSimulationInput(const Document& document,
                                         const ResourceProvider& resourceProvider);
}
