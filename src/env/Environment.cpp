// ------------------------------------------------
// Environment.hppの実装
// ------------------------------------------------

#include "env/Environment.hpp"

#include "utils/JsonUtils.hpp"

Environment::Environment(const boost::property_tree::ptree& specJson) :
    place(JsonUtils::GetValue<std::string>(specJson, "environment.place")),
    magneticDeclination(JsonUtils::GetOptional<double>(specJson, "environment.magnetic_declination")),
    railLength(JsonUtils::GetValueExc<double>(specJson, "environment.rail_len")),
    railAzimuth(JsonUtils::GetValueExc<double>(specJson, "environment.rail_azi")),
    railElevation(JsonUtils::GetValueExc<double>(specJson, "environment.rail_elev")) {}
