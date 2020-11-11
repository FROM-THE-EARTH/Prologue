#pragma once

namespace Constant {
	//math
	constexpr double PI = 3.141592653589793;

	//rocket
	constexpr double Cl = 1.0;

	//nature
	constexpr double G = 9.80665;//Base gravity [m/s^2]
	constexpr double EarthRadius = 6378.137e3;//Earth radius [m]
	constexpr double GasConstant = 287.0;//Gas constant of dry air [J/Kg*K]
	constexpr double SeaPressure = 1.01325e+5;//Air pressure at sea level [Pa]
	constexpr double AbsoluteZero = -273.15;//kelvin<->celsius [K]
}