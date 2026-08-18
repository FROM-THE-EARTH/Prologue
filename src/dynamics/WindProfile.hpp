// ------------------------------------------------
// 実測風データの補間
// ------------------------------------------------

#pragma once

#include <vector>

#include "math/Vector3D.hpp"

struct WindData {
    double geometricHeight = 0;  // Above the launch point [m]
    double speed           = 0;  // [m/s]
    double direction       = 0;  // Clockwise from north [deg]
};

class WindProfile {
    std::vector<WindData> m_data;

public:
    explicit WindProfile(std::vector<WindData> data);

    Vector3D windAt(double geometricHeight) const;

private:
    static Vector3D toWindVector(const WindData& data);
};
