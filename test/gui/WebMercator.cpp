#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "gui/map/WebMercator.hpp"

TEST_CASE("Web Mercator follows XYZ tile axis directions", "[gui][map]") {
    using namespace gui::webMercator;
    REQUIRE(project({0.0, 0.0}).x() == Approx(0.5));
    REQUIRE(project({0.0, 0.0}).y() == Approx(0.5));
    REQUIRE(project({180.0, 0.0}).x() == Approx(1.0));
    REQUIRE(project({-180.0, 0.0}).x() == Approx(0.0));
    REQUIRE(project({0.0, 45.0}).y() < 0.5);
    REQUIRE(project({0.0, -45.0}).y() > 0.5);
    REQUIRE(worldPixels(0) == 256.0);
    REQUIRE(worldPixels(14) == 4194304.0);
}

TEST_CASE("Web Mercator preserves geographic positions within its latitude range", "[gui][map]") {
    using namespace gui::webMercator;
    for (const auto& point : {QPointF(140.012, 40.244), QPointF(139.76, 35.68),
                              QPointF(-77.03, 38.90), QPointF(0.0, -80.0)}) {
        const auto restored = unproject(project(point));
        REQUIRE(restored.x() == Approx(point.x()).margin(1e-10));
        REQUIRE(restored.y() == Approx(point.y()).margin(1e-10));
    }
    REQUIRE(project({0.0, 90.0}).y() == Approx(0.0).margin(1e-14));
    REQUIRE(project({0.0, -90.0}).y() == Approx(1.0).margin(1e-14));
    REQUIRE(unproject(project({0.0, 90.0})).y() == Approx(MaximumLatitude));
}
