// ------------------------------------------------
// Environment.hppの実装
// ------------------------------------------------

#include "env/Environment.hpp"

#include <utility>

Environment::Environment(std::string place, double railLength, double railAzimuth, double railElevation) :
    place(std::move(place)), railLength(railLength), railAzimuth(railAzimuth), railElevation(railElevation) {}
