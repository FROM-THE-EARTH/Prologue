#pragma once

#include <filesystem>
#include <string>

#include <QString>

#include "project/ProjectDocument.hpp"

namespace gui {
    std::filesystem::path FilePath(const QString& text);
    QString PathText(const std::filesystem::path& path);
    void WriteAtomically(const QString& file, const std::string& contents);
    void RebaseResources(project::Document& document,
                         const std::filesystem::path& oldProjectFile,
                         const std::filesystem::path& newProjectFile);
}
