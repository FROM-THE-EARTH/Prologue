#define CATCH_CONFIG_MAIN

#include <cmath>
#include <numbers>

#include "catch2/catch.hpp"

#include "io/ApplicationSettingsReader.hpp"
#include "io/MeasuredWindProfileReader.hpp"

TEST_CASE("Application settings are converted to typed values", "[io][settings]") {
    const auto readResult = ApplicationSettingsReader::Read("data/wind_model/prologue.settings.json");
    const auto& settings = readResult.value;

    REQUIRE(readResult.diagnostics.empty());
    REQUIRE_FALSE(settings.execution.multiThread);
    REQUIRE(settings.execution.threadCount == 1);
    REQUIRE(settings.solver.timeStep == 0.001);
    REQUIRE(settings.scatter.windSpeedMin == 0.0);
    REQUIRE(settings.scatter.windSpeedMax == 0.0);
    REQUIRE(settings.scatter.windDirectionInterval == 30.0);
    REQUIRE(settings.result.precision == 8);
    REQUIRE(settings.solver.resultStepSaveInterval == 1);
    REQUIRE(settings.solver.wind.type == WindModelType::OnlyPowerLow);
    REQUIRE(settings.solver.wind.powerConstant == 2.0);
    REQUIRE(settings.solver.wind.powerLowBaseAltitude == 2.0);
    REQUIRE_FALSE(settings.solver.wind.measuredProfile.has_value());
    REQUIRE(settings.measuredWindFilename == "unused.csv");
    REQUIRE(settings.solver.atmosphere.basePressure == 101325.0);
    REQUIRE(settings.solver.atmosphere.baseTemperature == 15.0);
}

TEST_CASE("Measured wind CSV is converted to a declination-adjusted profile", "[io][wind]") {
    const auto profile = MeasuredWindProfileReader::Read("../application/input/wind/sample.csv");

    REQUIRE(profile.directionReference() == DirectionReference::MagneticNorth);
    const auto wind = profile.windAt(363.0, 10.0);
    const double radians = 148.0 * std::numbers::pi / 180.0;
    REQUIRE(wind.x == Approx(-std::sin(radians) * 4.0).margin(1e-12));
    REQUIRE(wind.y == Approx(-std::cos(radians) * 4.0).margin(1e-12));
    REQUIRE(wind.z == 0.0);
}
