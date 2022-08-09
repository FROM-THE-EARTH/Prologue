// ------------------------------------------------
// 諸元JSONファイルenvironmentキーのインターフェース
// ------------------------------------------------

#pragma once

#include <boost/property_tree/ptree.hpp>
#include <optional>
#include <string>

struct Environment {
    std::string place;
    std::optional<double> magneticDeclination;
    double railLength;
    double railAzimuth;
    double railElevation;

    void initialize(const boost::property_tree::ptree& specJson);
};
