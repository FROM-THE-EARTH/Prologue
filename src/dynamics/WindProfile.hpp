// ------------------------------------------------
// 実測風データの補間
// ------------------------------------------------

#pragma once

#include <vector>

#include "math/Vector3D.hpp"

enum class DirectionReference {
    TrueNorth,
    MagneticNorth,
};

struct WindDirection {
    double degrees = 0.0;
    DirectionReference reference = DirectionReference::TrueNorth;
};

double ResolveTrueNorthDirection(const WindDirection& direction, double magneticDeclination);

struct WindData {
    double geometricHeight = 0;  // Above the launch point [m]
    double speed           = 0;  // [m/s]
    double direction       = 0;  // Clockwise from the WindProfile direction reference [deg]
};

class WindProfile {
    std::vector<WindData> m_data;
    DirectionReference m_directionReference;

public:
    explicit WindProfile(std::vector<WindData> data, DirectionReference directionReference);

    [[nodiscard]] DirectionReference directionReference() const {
        return m_directionReference;
    }

    Vector3D windAt(double geometricHeight, double magneticDeclination) const;

private:
    Vector3D toWindVector(const WindData& data, double magneticDeclination) const;
};
