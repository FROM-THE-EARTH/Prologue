#pragma once
#include "SpecJson.h"
#include <vector>

namespace SpecJsonReader {

	SpecJson ReadJson(const std::string& filename);

	bool IsMultipleRocket(const std::string& filename);
}
