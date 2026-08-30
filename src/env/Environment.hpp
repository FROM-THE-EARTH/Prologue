// ------------------------------------------------
// 諸元JSONファイルenvironmentキーのインターフェース
// ------------------------------------------------

#pragma once

#include <string>

struct LaunchSite {
    std::string name;
    double latitude = 0.0;
    double longitude = 0.0;
    int coordinateZone = 0;
    double magneticDeclination = 0.0;
};

struct LaunchRail {
    double length = 0.0;
    double azimuth = 0.0;  // Input: clockwise from magnetic north [deg]
    double elevation = 0.0;
};

struct Environment {
    LaunchSite launchSite;
    LaunchRail launchRail;
};
