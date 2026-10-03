#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

#include "rocket/AeroCoefficient.hpp"
#include "rocket/Engine.hpp"

TEST_CASE("Typed thrust input cannot expose an invalid interpolation range", "[core][engine]") {
    REQUIRE_THROWS_AS(Engine({{.time = 0.0, .thrust = 10.0}}), std::invalid_argument);
    REQUIRE(Engine(std::vector<ThrustData>{}).thrustAt(0.0, 101325.0) == 0.0);

    const Engine engine({{.time = 0.0, .thrust = 0.0}, {.time = 1.0, .thrust = 10.0}});
    REQUIRE(engine.thrustAt(0.5, 101325.0) == 5.0);
}

TEST_CASE("Uninitialized aerodynamic input reports an error instead of indexing empty data", "[core][aero]") {
    REQUIRE_THROWS_AS(AeroCoefficientStorage({}, true), std::invalid_argument);
    REQUIRE_THROWS_AS(AeroCoefficientStorage{}.valuesIn(10.0, 0.0, false), std::invalid_argument);

    AeroCoefficientStorage storage;
    storage.init(0.7, 0.0, 0.3, 0.4, 0.0, 2.0);
    REQUIRE(storage.valuesIn(10.0, 0.0, false).Cd == 0.3);
}
