#pragma once

#include <vector>

#include "rocket/RocketSpec.hpp"

namespace RocketSpecReader {

    RocketSpec ReadJson(const std::string& filename);

    bool IsMultipleRocket(const std::string& filename);
}
