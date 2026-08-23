#pragma once

#include <filesystem>

#include "config/ApplicationSettings.hpp"
#include "io/ReadResult.hpp"

namespace ApplicationSettingsReader {
    InputIO::ReadResult<ApplicationSettings> Read(const std::filesystem::path& settingsFile);
}
