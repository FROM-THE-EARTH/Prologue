#define CATCH_CONFIG_MAIN

#include "catch2/catch.hpp"
#include "dynamics/StandardAtmosphere1976.hpp"
#include "dynamics/WindModel.hpp"

namespace {
    void RequireVector(const Vector3D& actual, const Vector3D& expected) {
        REQUIRE(actual.x == Approx(expected.x).margin(1e-10));
        REQUIRE(actual.y == Approx(expected.y).margin(1e-10));
        REQUIRE(actual.z == Approx(expected.z).margin(1e-10));
    }
}

TEST_CASE("Wind model samples without retaining the evaluation height", "[dynamics][wind]") {
    const WindModel model(4.0, 90.0, 0.0);

    const auto lowSample  = model.sampleAt(2.0);
    const auto highSample = model.sampleAt(8.0);

    RequireVector(lowSample.wind, Vector3D(-4.0, 0.0, 0.0));
    RequireVector(highSample.wind, Vector3D(-8.0, 0.0, 0.0));
    RequireVector(lowSample.wind, Vector3D(-4.0, 0.0, 0.0));
}

TEST_CASE("Wind model returns atmosphere values for the sampled height", "[dynamics][wind]") {
    const WindModel model(4.0, 90.0, 0.0);
    const auto sample = model.sampleAt(100.0);
    const auto atmosphere = StandardAtmosphere1976::calculate(100.0, 101325.0, 15.0);

    REQUIRE(sample.density == atmosphere.density);
    REQUIRE(sample.gravity == atmosphere.gravity);
    REQUIRE(sample.pressure == atmosphere.pressure);
    REQUIRE(sample.temperature == atmosphere.temperature);
}
