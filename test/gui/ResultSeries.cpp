#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include <string>

#include "geography/GeographicResult.hpp"
#include "gui/plot/ResultSeries.hpp"
#include "misc/Constant.hpp"

using namespace gui::plot;

TEST_CASE("Detail plotting retains CLI quantities and explicit angle units", "[gui][plot]") {
    SimulationStep step;
    step.gen_timeFromLaunch = 7.5;
    step.gen_elapsedTime = 2.5;
    step.rocket_mass = 2.0;
    step.rocket_attackAngle = Constant::PI / 6.0;
    step.rocket_velocity = {3.0, 4.0, 0.0};
    step.rocket_airspeed_b = {0.0, 0.0, 12.0};
    step.rocket_force_b = {24.0, 3.0, 4.0};
    step.Fst = 18.0;
    step.dynamicPressure = 125.0;
    const GeographicPosition position{35.5, 139.5};

    REQUIRE(DetailValue(step, DetailQuantity::TimeFromLaunch) == 7.5);
    REQUIRE(DetailValue(step, DetailQuantity::ElapsedTime) == 2.5);
    REQUIRE(DetailValue(step, DetailQuantity::GroundSpeed) == 5.0);
    REQUIRE(DetailValue(step, DetailQuantity::Airspeed) == 12.0);
    REQUIRE(DetailValue(step, DetailQuantity::LongitudinalAcceleration) == 12.0);
    REQUIRE(DetailValue(step, DetailQuantity::NormalForce) == 5.0);
    REQUIRE(DetailValue(step, DetailQuantity::StaticMargin) == 18.0);
    REQUIRE(DetailValue(step, DetailQuantity::DynamicPressure) == 125.0);
    REQUIRE(DetailValue(step, DetailQuantity::AttackAngleRadians) == step.rocket_attackAngle);
    REQUIRE(DetailValue(step, DetailQuantity::AttackAngleDegrees) == Approx(30.0));
    REQUIRE(DetailValue(step, DetailQuantity::Latitude, &position) == 35.5);
    REQUIRE_THROWS_AS(DetailValue(step, DetailQuantity::Latitude), std::invalid_argument);

    bool hasRadians = false, hasDegrees = false;
    for (const auto& field : DetailQuantities()) {
        if (field.quantity == DetailQuantity::AttackAngleRadians) hasRadians = std::string(field.unit) == "rad";
        if (field.quantity == DetailQuantity::AttackAngleDegrees) hasDegrees = std::string(field.unit) == "deg";
    }
    REQUIRE(hasRadians);
    REQUIRE(hasDegrees);
    REQUIRE(AxisLabel("Static margin Fst", "%") == "Static margin Fst [%]");
    REQUIRE(AxisLabel("Cnp", "") == "Cnp");
}

TEST_CASE("Wind profile magnitude includes all ENU components", "[gui][plot]") {
    SimulationStep step;
    step.air_wind = {3.0, -4.0, 12.0};
    REQUIRE(DetailValue(step, DetailQuantity::WindSpeed) == 13.0);
}

TEST_CASE("Summary plotting uses stored run values and the selected body's final position", "[gui][plot]") {
    SimulationResult result;
    result.windSpeed = 4.5;
    result.windDirection = {.degrees = 30.0, .reference = DirectionReference::MagneticNorth};
    result.maxAltitude = 800.0;
    result.maxDynamicPressureDuringRising = 2500.0;
    result.launchClearVelocity = {0.0, 0.0, 20.0};
    result.bodyFinalPositions = {{300.0, 400.0, -0.1}, {6.0, 8.0, -0.2}};
    const GeographicPosition finalPosition{40.5, 140.5};

    REQUIRE(SummaryValue(result, SummaryQuantity::WindSpeed) == 4.5);
    REQUIRE(SummaryValue(result, SummaryQuantity::WindDirection) == 30.0);
    REQUIRE(SummaryValue(result, SummaryQuantity::PeakAltitude) == 800.0);
    REQUIRE(SummaryValue(result, SummaryQuantity::MaxDynamicPressure) == 2500.0);
    REQUIRE(SummaryValue(result, SummaryQuantity::LaunchClearSpeed) == 20.0);
    REQUIRE(SummaryValue(result, SummaryQuantity::FinalDownrange, 1) == 10.0);
    REQUIRE(SummaryValue(result, SummaryQuantity::FinalAltitude, 1) == -0.2);
    REQUIRE(SummaryValue(result, SummaryQuantity::FinalLongitude, 1, &finalPosition) == 140.5);
    REQUIRE_THROWS_AS(SummaryValue(result, SummaryQuantity::FinalEast, 2), std::out_of_range);
    REQUIRE(NeedsBody(SummaryQuantity::FinalEast));
    REQUIRE_FALSE(NeedsBody(SummaryQuantity::WindDirection));
    REQUIRE(NeedsGeography(SummaryQuantity::FinalLatitude));
    REQUIRE_FALSE(NeedsGeography(SummaryQuantity::PeakAltitude));

    for (const auto& field : SummaryQuantities()) {
        if (field.quantity == SummaryQuantity::MaxDynamicPressure) {
            REQUIRE(AxisLabel(field.label, field.unit) == "Maximum dynamic pressure during ascent [Pa]");
        }
    }
}
