#pragma once

#include <filesystem>
#include <string>

#include "config/ApplicationSettings.hpp"

namespace ApplicationSettingsWriter {
    // Uses the same prologue.settings.json format as ApplicationSettingsReader.
    std::string Serialize(const ApplicationSettings& settings);
    void Write(const std::filesystem::path& file, const ApplicationSettings& settings);
}
