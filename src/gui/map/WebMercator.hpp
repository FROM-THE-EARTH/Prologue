// ------------------------------------------------
// 地図タイル表示にだけ使う Web Mercator 座標変換
// ------------------------------------------------

#pragma once

#include <algorithm>
#include <cmath>
#include <numbers>

#include <QPointF>

namespace gui::webMercator {
    inline constexpr double MaximumLatitude = 85.0511287798066;

    // QPointF uses longitude for x and latitude for y. The map world is [0, 1].
    inline QPointF project(const QPointF& longitudeLatitude) {
        const double latitude = std::clamp(longitudeLatitude.y(), -MaximumLatitude, MaximumLatitude);
        const double radians = latitude * std::numbers::pi / 180.0;
        return {(longitudeLatitude.x() + 180.0) / 360.0,
                (1.0 - std::asinh(std::tan(radians)) / std::numbers::pi) / 2.0};
    }

    inline QPointF unproject(const QPointF& normalizedPosition) {
        return {normalizedPosition.x() * 360.0 - 180.0,
                std::atan(std::sinh(std::numbers::pi * (1.0 - 2.0 * normalizedPosition.y())))
                    * 180.0 / std::numbers::pi};
    }

    inline double worldPixels(int zoom) {
        return std::ldexp(256.0, zoom);
    }
}
