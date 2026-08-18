#define CATCH_CONFIG_MAIN

#include "dynamics/StandardAtmosphere1976.hpp"

#include <cmath>
#include <limits>

#include "catch2/catch.hpp"

namespace {
    constexpr double EffectiveEarthRadius = 6356766.0;

    double toGeometricHeight(double geopotentialHeight) {
        return EffectiveEarthRadius * geopotentialHeight / (EffectiveEarthRadius - geopotentialHeight);
    }
}

TEST_CASE("U.S. Standard Atmosphere 1976 reference values", "[dynamics][atmosphere]") {
    struct ReferenceValue {
        double geopotentialHeight;
        double temperatureKelvin;
        double pressure;
        double density;
    };

    const ReferenceValue referenceValues[] = {
        {0.0, 288.15, 101325.0, 1.22500},
        {5000.0, 255.65, 54019.9, 0.736116},
        {11000.0, 216.65, 22632.1, 0.363918},
        {15000.0, 216.65, 12044.6, 0.193674},
        {20000.0, 216.65, 5474.89, 0.0880349},
        {25000.0, 221.65, 2511.02, 0.0394658},
        {30000.0, 226.65, 1171.87, 0.0180119},
        {32000.0, 228.65, 868.019, 0.0132250},
    };

    for (const auto& expected : referenceValues) {
        const auto actual = StandardAtmosphere1976::calculate(toGeometricHeight(expected.geopotentialHeight));
        REQUIRE(actual.geopotentialHeight == Approx(expected.geopotentialHeight).margin(1e-8));
        REQUIRE(actual.temperature + 273.15 == Approx(expected.temperatureKelvin).margin(1e-8));
        REQUIRE(actual.pressure == Approx(expected.pressure).epsilon(1e-5));
        REQUIRE(actual.density == Approx(expected.density).epsilon(2e-5));
    }
}

TEST_CASE("Atmosphere layer boundaries are continuous", "[dynamics][atmosphere]") {
    for (const double boundary : {11000.0, 20000.0}) {
        const auto below = StandardAtmosphere1976::calculate(toGeometricHeight(boundary - 1e-3));
        const auto at    = StandardAtmosphere1976::calculate(toGeometricHeight(boundary));
        const auto above = StandardAtmosphere1976::calculate(toGeometricHeight(boundary + 1e-3));

        REQUIRE(below.temperature == Approx(at.temperature).margin(1e-5));
        REQUIRE(above.temperature == Approx(at.temperature).margin(1e-5));
        REQUIRE(below.pressure == Approx(at.pressure).epsilon(1e-6));
        REQUIRE(above.pressure == Approx(at.pressure).epsilon(1e-6));
    }
}

TEST_CASE("Pressure and density decrease through the supported range", "[dynamics][atmosphere]") {
    auto previous = StandardAtmosphere1976::calculate(0.0);
    for (double geopotentialHeight = 100.0;
         geopotentialHeight <= StandardAtmosphere1976::MaximumGeopotentialHeight;
         geopotentialHeight += 100.0) {
        const auto current = StandardAtmosphere1976::calculate(toGeometricHeight(geopotentialHeight));
        REQUIRE(current.pressure < previous.pressure);
        REQUIRE(current.density < previous.density);
        previous = current;
    }
}

TEST_CASE("The documented altitude limit is inclusive", "[dynamics][atmosphere]") {
    REQUIRE_NOTHROW(StandardAtmosphere1976::calculate(toGeometricHeight(32000.0)));
    REQUIRE_THROWS_AS(StandardAtmosphere1976::calculate(toGeometricHeight(32000.1)), std::out_of_range);
    REQUIRE_THROWS_AS(StandardAtmosphere1976::calculate(std::numeric_limits<double>::infinity()),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(StandardAtmosphere1976::calculate(0.0, 101325.0, -250.0), std::invalid_argument);
}

TEST_CASE("Custom sea-level settings preserve layer continuity", "[dynamics][atmosphere]") {
    const auto below = StandardAtmosphere1976::calculate(toGeometricHeight(10999.999), 100000.0, 20.0);
    const auto above = StandardAtmosphere1976::calculate(toGeometricHeight(11000.001), 100000.0, 20.0);

    REQUIRE(below.temperature == Approx(above.temperature).margin(2e-5));
    REQUIRE(below.pressure == Approx(above.pressure).epsilon(1e-6));
}
