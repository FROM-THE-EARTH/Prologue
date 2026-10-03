#include "MapSeriesBuilder.hpp"

#include <algorithm>
#include <map>

#include "geography/GeographicResult.hpp"

namespace gui {
    namespace {
        QColor BodyColor(size_t index) {
            const QColor colors[] = {QColor("#2364aa"), QColor("#ed7d31"), QColor("#7c4d9d")};
            return colors[index % 3];
        }
        QPointF Point(const GeographicPosition& point) { return {point.longitude, point.latitude}; }
    }

    std::vector<MapSeries> BuildMapSeries(const RunResults& results) {
        const auto& site = results.environment.launchSite;
        const GeoCoordinate coordinate(site.latitude, site.longitude, site.coordinateZone);
        std::vector<MapSeries> series{{QStringLiteral("Launch site"), QColor("#21834a"),
                                      {{site.longitude, site.latitude}}, false, true}};
        if (results.mode == SimulationMode::Detail) {
            const auto& result = results.simulations.at(0);
            const auto geographic = GeographicResultAdapter::Convert(result, coordinate);
            for (size_t body = 0; body < geographic.bodyStepPositions.size(); ++body) {
                MapSeries trajectory{QString("Body %1 trajectory").arg(body + 1), BodyColor(body)};
                for (const auto& point : geographic.bodyStepPositions[body]) trajectory.coordinates.push_back(Point(point));
                series.emplace_back(std::move(trajectory));
            }
            for (size_t body = 0; body < geographic.bodyFinalPositions.size(); ++body) {
                const QString kind = result.bodyFinalPositions[body].z > 0.0 ? "separation" : "landing";
                series.push_back({QString("Body %1 %2").arg(body + 1).arg(kind), BodyColor(body),
                                  {Point(geographic.bodyFinalPositions[body])}, false, true});
            }
        } else {
            using RingKey = std::pair<size_t, double>;
            std::map<RingKey, std::vector<std::pair<double, QPointF>>> rings;
            for (const auto& result : results.simulations) {
                const auto geographic = GeographicResultAdapter::Convert(result, coordinate);
                for (size_t body = 0; body < geographic.bodyFinalPositions.size(); ++body) {
                    // A body released above ground has a separation location, not a landing.
                    if (result.bodyFinalPositions[body].z > 0.0) continue;
                    rings[{body, result.windSpeed}].emplace_back(
                        result.windDirection.degrees, Point(geographic.bodyFinalPositions[body]));
                }
            }
            size_t colorIndex = 0;
            for (auto& [key, positions] : rings) {
                std::sort(positions.begin(), positions.end(), [](const auto& first, const auto& second) {
                    return first.first < second.first;
                });
                const auto [body, speed] = key;
                const QColor color = QColor::fromHsv(static_cast<int>((colorIndex++ * 47) % 360), 190, 180);
                MapSeries ring{QString("Body %1 / %2 m/s").arg(body + 1).arg(speed), color, {}, true, false};
                for (const auto& [direction, point] : positions) {
                    static_cast<void>(direction);
                    ring.coordinates.push_back(point);
                }
                series.push_back(ring);
                ring.closed = false;
                ring.points = true;
                series.push_back(std::move(ring));
            }
        }
        return series;
    }
}
