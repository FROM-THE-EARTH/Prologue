// ------------------------------------------------
// 既存JSON形式から型付き入力への変換
// ------------------------------------------------

#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "core/SimulationInput.hpp"
#include "io/ReadResult.hpp"

namespace SimulationInputReader {
    class Document {
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        explicit Document(const std::filesystem::path& specificationFile);
        ~Document();

        Document(Document&&) noexcept;
        Document& operator=(Document&&) noexcept;

        Document(const Document&) = delete;
        Document& operator=(const Document&) = delete;

        std::string specificationName() const;

        bool isMultipleRocket() const;

        InputIO::ReadResult<SimulationInput> toSimulationInput() const;
    };
}
