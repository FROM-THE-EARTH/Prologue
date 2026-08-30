// ------------------------------------------------
// 保存可能なプロジェクト諸元JSONの読み書き
// ------------------------------------------------

#pragma once

#include <filesystem>

#include "project/ProjectDocument.hpp"

namespace ProjectJsonSerializer {
    project::Document Load(const std::filesystem::path& file);
    void Save(const std::filesystem::path& file, const project::Document& document);
}
