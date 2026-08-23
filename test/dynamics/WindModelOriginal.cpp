#define CATCH_CONFIG_MAIN

#include <cmath>

#include "catch2/catch.hpp"
#include "dynamics/WindModel.hpp"

namespace {
    WindModel MakeOriginalModel(double speed, double direction) {
        WindModelSettings wind;
        wind.type = WindModelType::Original;
        wind.powerConstant = 6.0;
        wind.powerLowBaseAltitude = 2.0;
        return WindModel(wind,
                         AtmosphereSettings{},
                         speed,
                         WindDirection{.degrees = direction,
                                       .reference = DirectionReference::MagneticNorth},
                         0.0);
    }
}

TEST_CASE("Original wind model keeps Ekman wind horizontal", "[dynamics][wind]") {
    const WindModel model = MakeOriginalModel(4.0, 0.0);

    for (const double height : {300.0, 650.0, 999.999, 1000.0}) {
        REQUIRE(model.sampleAt(height).wind.z == 0.0);
    }
}

TEST_CASE("Original wind model maps Ekman components to east and north", "[dynamics][wind]") {
    constexpr double GeostrophicWind = 15.0;
    constexpr double SurfaceLayerLimit = 300.0;
    constexpr double height = 650.0;

    const WindModel model = MakeOriginalModel(0.0, 0.0);
    const auto wind = model.sampleAt(height).wind;

    const double k = (height - SurfaceLayerLimit) / (SurfaceLayerLimit * std::sqrt(2.0));
    const double expectedEast = GeostrophicWind * (1.0 - std::exp(-k) * std::cos(k));
    const double expectedNorth = GeostrophicWind * std::exp(-k) * std::sin(k);

    REQUIRE(wind.x == Approx(expectedEast).margin(1e-12));
    REQUIRE(wind.y == Approx(expectedNorth).margin(1e-12));
    REQUIRE(wind.z == 0.0);
}
