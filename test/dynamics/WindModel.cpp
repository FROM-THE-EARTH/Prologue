#define CATCH_CONFIG_MAIN

#include "catch2/catch.hpp"
#include "dynamics/StandardAtmosphere1976.hpp"
#include "dynamics/WindModel.hpp"

namespace {
    WindModel MakePowerLawModel(double speed, double direction) {
        WindModelSettings wind;
        wind.type = WindModelType::OnlyPowerLow;
        wind.powerConstant = 2.0;
        wind.powerLowBaseAltitude = 2.0;
        return WindModel(wind,
                         AtmosphereSettings{},
                         speed,
                         WindDirection{.degrees = direction,
                                       .reference = DirectionReference::MagneticNorth},
                         0.0);
    }

    void RequireVector(const Vector3D& actual, const Vector3D& expected) {
        REQUIRE(actual.x == Approx(expected.x).margin(1e-10));
        REQUIRE(actual.y == Approx(expected.y).margin(1e-10));
        REQUIRE(actual.z == Approx(expected.z).margin(1e-10));
    }
}

TEST_CASE("Wind model samples without retaining the evaluation height", "[dynamics][wind]") {
    const WindModel model = MakePowerLawModel(4.0, 90.0);

    const auto lowSample  = model.sampleAt(2.0);
    const auto highSample = model.sampleAt(8.0);

    RequireVector(lowSample.wind, Vector3D(-4.0, 0.0, 0.0));
    RequireVector(highSample.wind, Vector3D(-8.0, 0.0, 0.0));
    RequireVector(lowSample.wind, Vector3D(-4.0, 0.0, 0.0));
}

TEST_CASE("Wind model returns atmosphere values for the sampled height", "[dynamics][wind]") {
    const WindModel model = MakePowerLawModel(4.0, 90.0);
    const auto sample = model.sampleAt(100.0);
    const auto atmosphere = StandardAtmosphere1976::calculate(100.0, 101325.0, 15.0);

    REQUIRE(sample.density == atmosphere.density);
    REQUIRE(sample.gravity == atmosphere.gravity);
    REQUIRE(sample.pressure == atmosphere.pressure);
    REQUIRE(sample.temperature == atmosphere.temperature);
}

TEST_CASE("Measured wind profile is supplied as typed input", "[dynamics][wind]") {
    WindModelSettings wind;
    wind.type = WindModelType::Real;
    wind.measuredProfile.emplace(
        std::vector<WindData>{{.geometricHeight = 100.0, .speed = 10.0, .direction = 90.0}},
        DirectionReference::TrueNorth);

    const WindModel model(
        wind,
        AtmosphereSettings{},
        0.0,
        WindDirection{.degrees = 0.0, .reference = DirectionReference::TrueNorth},
        0.0);
    RequireVector(model.sampleAt(50.0).wind, Vector3D(-5.0, 0.0, 0.0));
}

TEST_CASE("Wind model resolves magnetic and true-north directions consistently", "[dynamics][wind]") {
    WindModelSettings wind;
    wind.type = WindModelType::OnlyPowerLow;
    wind.powerConstant = 2.0;
    wind.powerLowBaseAltitude = 2.0;

    const WindModel magnetic(
        wind,
        AtmosphereSettings{},
        4.0,
        WindDirection{.degrees = 80.0, .reference = DirectionReference::MagneticNorth},
        10.0);
    const WindModel trueNorth(
        wind,
        AtmosphereSettings{},
        4.0,
        WindDirection{.degrees = 90.0, .reference = DirectionReference::TrueNorth},
        10.0);

    RequireVector(magnetic.sampleAt(2.0).wind, Vector3D(-4.0, 0.0, 0.0));
    RequireVector(trueNorth.sampleAt(2.0).wind, Vector3D(-4.0, 0.0, 0.0));
}

TEST_CASE("Measured wind model resolves its profile reference in core", "[dynamics][wind]") {
    WindModelSettings magneticSettings;
    magneticSettings.type = WindModelType::Real;
    magneticSettings.measuredProfile.emplace(
        std::vector<WindData>{{.geometricHeight = 100.0, .speed = 4.0, .direction = 80.0}},
        DirectionReference::MagneticNorth);

    WindModelSettings trueNorthSettings;
    trueNorthSettings.type = WindModelType::Real;
    trueNorthSettings.measuredProfile.emplace(
        std::vector<WindData>{{.geometricHeight = 100.0, .speed = 4.0, .direction = 90.0}},
        DirectionReference::TrueNorth);

    const WindDirection unusedGroundDirection{
        .degrees = 0.0,
        .reference = DirectionReference::TrueNorth,
    };
    const WindModel magnetic(
        magneticSettings, AtmosphereSettings{}, 0.0, unusedGroundDirection, 10.0);
    const WindModel trueNorth(
        trueNorthSettings, AtmosphereSettings{}, 0.0, unusedGroundDirection, 10.0);

    RequireVector(magnetic.sampleAt(100.0).wind, Vector3D(-4.0, 0.0, 0.0));
    RequireVector(trueNorth.sampleAt(100.0).wind, Vector3D(-4.0, 0.0, 0.0));
}
