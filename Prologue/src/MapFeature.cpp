#include "MapFeature.h"

#include <string>


namespace MapFeature {
	Feature GetMapFromName(const char* name) {
		if (strcmp(name, "nosiro_sea")) {
			return NoshiroSea;
		}

		if (strcmp(name, "nosiro_land")) {
			return NoshiroLand;
		}

		if (strcmp(name, "izu_sea")) {
			return IzuSea;
		}

		if (strcmp(name, "izu_land")) {
			return IzuLand;
		}

		return Unknown;
	}
}