// ------------------------------------------------
// 実測風データの補間
// ------------------------------------------------

#include "WindProfile.hpp"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <numbers>
#include <stdexcept>
#include <string>
#include <utility>

WindProfile::WindProfile(std::vector<WindData> data) : m_data(std::move(data)) {
    if (m_data.empty()) {
        throw std::invalid_argument{"Wind profile must contain at least one data point."};
    }

    for (size_t i = 0; i < m_data.size(); i++) {
        const auto& current = m_data[i];
        if (!std::isfinite(current.geometricHeight) || !std::isfinite(current.speed)
            || !std::isfinite(current.direction)) {
            throw std::invalid_argument{"Wind profile values must be finite."};
        }
        if (current.geometricHeight < 0.0) {
            throw std::invalid_argument{"Wind profile heights must not be below ground level."};
        }
        if (current.speed < 0.0) {
            throw std::invalid_argument{"Wind profile speeds must not be negative."};
        }
        if (current.geometricHeight == 0.0 && current.speed != 0.0) {
            throw std::invalid_argument{"Wind speed at ground level must be zero."};
        }
        if (i > 0 && m_data[i - 1].geometricHeight >= current.geometricHeight) {
            throw std::invalid_argument{"Wind profile heights must be strictly increasing."};
        }
    }
}

Vector3D WindProfile::windAt(double geometricHeight) const {
    if (!std::isfinite(geometricHeight)) {
        throw std::invalid_argument{"Geometric height must be finite."};
    }

    // The wind is zero at ground level. Between the ground and the lowest
    // observation, scaling the lowest observed vector preserves its direction.
    if (geometricHeight <= 0.0) {
        return Vector3D();
    }

    const auto upper = std::lower_bound(
        m_data.begin(),
        m_data.end(),
        geometricHeight,
        [](const WindData& data, double queryHeight) { return data.geometricHeight < queryHeight; });

    // No boundary condition is available above the highest observation, so
    // reject the query instead of introducing an extrapolation assumption.
    if (upper == m_data.end()) {
        throw std::out_of_range{"Geometric height " + std::to_string(geometricHeight)
                                + " m exceeds the highest wind observation at "
                                + std::to_string(m_data.back().geometricHeight) + " m."};
    }

    const Vector3D upperWind = toWindVector(*upper);
    if (upper == m_data.begin()) {
        return upperWind * (geometricHeight / upper->geometricHeight);
    }

    const auto lower         = std::prev(upper);
    const Vector3D lowerWind = toWindVector(*lower);
    const double interpolationRatio =
        (geometricHeight - lower->geometricHeight) / (upper->geometricHeight - lower->geometricHeight);
    return lowerWind + (upperWind - lowerWind) * interpolationRatio;
}

Vector3D WindProfile::toWindVector(const WindData& data) {
    const double rad = data.direction * std::numbers::pi / 180.0;
    return -Vector3D(std::sin(rad), std::cos(rad), 0.0) * data.speed;
}
