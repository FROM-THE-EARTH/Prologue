#include "FileActions.hpp"

#include <stdexcept>

#include <QSaveFile>

namespace gui {
    std::filesystem::path FilePath(const QString& text) {
        const std::string utf8 = text.toStdString();
        return std::filesystem::path(std::u8string(utf8.begin(), utf8.end()));
    }

    QString PathText(const std::filesystem::path& path) {
        const auto utf8 = path.generic_u8string();
        return QString::fromUtf8(reinterpret_cast<const char*>(utf8.data()),
                                 static_cast<qsizetype>(utf8.size()));
    }

    void WriteAtomically(const QString& file, const std::string& contents) {
        QSaveFile output(file);
        if (!output.open(QIODevice::WriteOnly)
            || output.write(contents.data(), static_cast<qint64>(contents.size())) != static_cast<qint64>(contents.size())
            || !output.commit()) {
            throw std::runtime_error{("Cannot save " + file + ": " + output.errorString()).toStdString()};
        }
    }

    void RebaseResources(project::Document& document,
                         const std::filesystem::path& oldProjectFile,
                         const std::filesystem::path& newProjectFile) {
        const auto oldDirectory = std::filesystem::absolute(oldProjectFile).parent_path();
        const auto newDirectory = std::filesystem::absolute(newProjectFile).parent_path();
        const auto rebase = [&](std::optional<std::filesystem::path>& path) {
            if (!path || path->is_absolute()) return;
            const auto absolute = (oldDirectory / *path).lexically_normal();
            const auto relative = absolute.lexically_relative(newDirectory);
            path = relative.empty() ? absolute : relative;
        };
        for (auto& body : document.bodies) {
            rebase(body.engine.thrustFile);
            rebase(body.aerodynamics.coefficientFile);
        }
    }
}
