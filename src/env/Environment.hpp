// ------------------------------------------------
// 諸元JSONファイルenvironmentキーのインターフェース
// ------------------------------------------------

#pragma once

#include <string>

struct Environment {
    std::string place;
    double railLength;
    double railAzimuth;  // Input: clockwise from magnetic north [deg]
    double railElevation;

    explicit Environment(std::string place, double railLength, double railAzimuth, double railElevation);
};
