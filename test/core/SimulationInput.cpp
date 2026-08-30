#define CATCH_CONFIG_MAIN

#include "core/SimulationInput.hpp"

#include <utility>
#include <vector>

#include "catch2/catch.hpp"
#include "math/Interpolation.hpp"

TEST_CASE("Typed simulation input owns resolved numerical data", "[core][input]") {
    BodySpecification body;
    body.engine = Engine({{.time = 0.0, .thrust = 0.0},
                          {.time = 1.0, .thrust = 100.0},
                          {.time = 2.0, .thrust = 0.0}});
    body.aeroCoefStorage = AeroCoefficientStorage(
        {{.airspeed = 0.0, .Cp = 1.0, .Cp_a = 0.0, .Cd_i = 0.5, .Cd_f = 0.4, .Cd_a2 = 0.0, .Cna = 10.0}},
        true);

    std::vector<BodySpecification> bodies;
    bodies.emplace_back(std::move(body));

    const SimulationInput input{
        .environment = {
            .launchSite = {.name = "test",
                           .latitude = 35.0,
                           .longitude = 139.0,
                           .coordinateZone = 9,
                           .magneticDeclination = 0.0},
            .launchRail = {.length = 5.0, .azimuth = 0.0, .elevation = 80.0},
        },
        .rocket      = RocketSpecification(std::move(bodies)),
    };

    REQUIRE(input.rocket.bodyCount() == 1);
    REQUIRE(input.rocket.bodySpec(0).engine.thrustAt(0.5, 101325.0) == Approx(50.0));
    REQUIRE(input.rocket.bodySpec(0).aeroCoefStorage.valuesIn(0.0, 0.0, false).Cd == Approx(0.5));
}

TEST_CASE("Typed rocket input is not limited to the legacy three-body layout", "[core][input]") {
    std::vector<BodySpecification> bodies(5);
    std::vector<SeparationSpecification> separations{
        {.sourceBodyIndex = 0, .productBodyIndices = {1, 2}},
        {.sourceBodyIndex = 2, .productBodyIndices = {3, 4}},
    };
    const RocketSpecification rocket(std::move(bodies), std::move(separations));

    REQUIRE(rocket.isMultiple());
    REQUIRE(rocket.bodyCount() == 5);
    REQUIRE(rocket.separations().size() == 2);
}

TEST_CASE("Core interpolation preserves the legacy zero-width fallback", "[core][math]") {
    REQUIRE(Interpolation::Linear(1.0, 2.0, 2.0, 3.0, 4.0) == 3.0);
}
