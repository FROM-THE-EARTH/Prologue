﻿// ------------------------------------------------
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

    Environment() = delete;

    explicit Environment(const boost::property_tree::ptree& specJson);
};
