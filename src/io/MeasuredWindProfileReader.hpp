#pragma once

#include <filesystem>

#include "dynamics/WindProfile.hpp"

namespace MeasuredWindProfileReader {
    WindProfile Read(const std::filesystem::path& windFile);
}
