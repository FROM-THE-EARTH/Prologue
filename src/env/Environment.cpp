// ------------------------------------------------
// Environment.hppの実装
// ------------------------------------------------

#include "env/Environment.hpp"

#include <boost/property_tree/ptree.hpp>

#include "utils/JsonUtils.hpp"

void Environment::initialize(const std::string& filename) {
    boost::property_tree::ptree pt;
    boost::property_tree::read_json("input/json/" + filename, pt);

    place               = JsonUtils::GetValue<std::string>(pt, "environment.place");
    magneticDeclination = JsonUtils::GetOptional<double>(pt, "environment.magnetic_declination");
    railLength          = JsonUtils::GetValueExc<double>(pt, "environment.rail_len");
    railAzimuth         = JsonUtils::GetValueExc<double>(pt, "environment.rail_azi");
    railElevation       = JsonUtils::GetValueExc<double>(pt, "environment.rail_elev");
}
