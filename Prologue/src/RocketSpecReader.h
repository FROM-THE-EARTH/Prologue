#pragma once
#include "RocketSpec.h"
#include <vector>

namespace RocketSpecReader {

	RocketSpec ReadJson(const std::string& filename);

	bool IsMultipleRocket(const std::string& filename);
}
