#pragma once

#include <string>

struct Environment {
    std::string place;
    double railLength;
    double railAzimuth;
    double railElevation;

    void initialize(const std::string& filename);
};
