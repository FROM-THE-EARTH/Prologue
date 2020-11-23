#pragma once

#include "Constant.h"

#include <math.h>
#include <string>


enum class Map {
	NOSIRO_SEA,
	NOSIRO_LAND,
	IZU_SEA,
	IZU_LAND,
	UNKNOWN
};


struct Feature {
private:
	double degPerLen_latitude;
	double degPerLen_longitude;

public:
	const Map map;
	const double latitude;//launchpoint [deg N]
	const double longitude;//launchpoint [deg E]

	Feature(Map _map, double _latitude, double _longitude)
		:map(_map),
		latitude(_latitude),
		longitude(_longitude)
	{
		degPerLen_latitude = 31.0 / 0.00027778;
		degPerLen_longitude = 6378150.0 * cos(latitude / 180.0 * Constant::PI) * 2.0 * Constant::PI / 360.0;
	}

	double latitudeAt(double length) const{//length: from here
		return latitude + length / degPerLen_latitude;
	}

	double longitudeAt(double length) const{//length: from here
		return longitude + length / degPerLen_longitude;
	}
};


namespace MapFeature {
	const Feature NoshiroSea{
		Map::NOSIRO_SEA,
		40.242865,
		140.010450
	};

	const Feature NoshiroLand{
		Map::NOSIRO_LAND,
		0,
		0
	};

	const Feature IzuSea{
		Map::IZU_SEA,
		0,
		0
	};

	const Feature IzuLand{
		Map::IZU_LAND,
		0,
		0
	};

	const Feature Unknown{
		Map::UNKNOWN,
		0,
		0
	};


	Feature GetMapFromName(const std::string& name);

}