// ------------------------------------------------
// 諸元JSONファイルenvironmentキーのインターフェース
// ------------------------------------------------

#pragma once

#include <optional>
#include <string>

struct Environment {
    std::string place;
    std::optional<double> magneticDeclination;
    double railLength;
    double railAzimuth;
    double railElevation;

    void initialize(const std::string& filename);
};
