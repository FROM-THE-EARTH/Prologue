// ------------------------------------------------
// 保存可能なプロジェクト諸元JSONの読み書き
// ------------------------------------------------

#pragma once

#include <filesystem>
#include <string>

#include "project/ProjectDocument.hpp"

namespace ProjectJsonSerializer {
    project::Document Load(const std::filesystem::path& file);
    std::string Serialize(const project::Document& document);
    void Save(const std::filesystem::path& file, const project::Document& document);
}
