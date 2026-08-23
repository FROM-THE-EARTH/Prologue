#define CATCH_CONFIG_MAIN

#include "io/SimulationInputReader.hpp"

#include "catch2/catch.hpp"

TEST_CASE("Single-body JSON is converted to typed input", "[io][input]") {
    const SimulationInputReader::Document document("input/spec/spec_single.json");
    REQUIRE_FALSE(document.isMultipleRocket());
    const auto readResult = document.toSimulationInput();
    const auto& input = readResult.value;

    REQUIRE_FALSE(readResult.diagnostics.empty());
    REQUIRE(document.specificationName() == "spec_single");
    REQUIRE(input.environment.place == "nosiro_sea");
    REQUIRE(input.environment.railLength == Approx(5.0));
    REQUIRE(input.environment.railAzimuth == Approx(-60.0));
    REQUIRE(input.environment.railElevation == Approx(70.0));

    REQUIRE(input.rocket.bodyCount() == 1);
    REQUIRE_FALSE(input.rocket.isMultiple());

    const auto& body = input.rocket.bodySpec(0);
    REQUIRE(body.length == Approx(2.01));
    REQUIRE(body.diameter == Approx(0.091));
    REQUIRE(body.massInitial == Approx(8.243));
    REQUIRE(body.massFinal == Approx(7.867));
    REQUIRE(body.parachutes.size() == 1);
    REQUIRE(body.parachutes[0].openingType == PARACHUTE_OPENING_TYPE_DETECT_PEAK);
    REQUIRE(body.parachutes[0].openingHeight == Approx(10.0));
    REQUIRE(body.parachutes[0].CdS == Approx(7.0));
    REQUIRE(body.transitions.size() == 2);
    REQUIRE(body.engine.combustionTime() == Approx(6.60));
    REQUIRE(body.engine.thrustAt(0.02, 101325.0) == Approx(305.23));

    const auto aero = body.aeroCoefStorage.valuesIn(0.0, 0.0, false);
    REQUIRE(aero.Cp == Approx(1.38));
    REQUIRE(aero.Cd == Approx(0.5));
    REQUIRE(aero.Cna == Approx(11.747));
}

TEST_CASE("Multi-body JSON preserves all current body configurations", "[io][input]") {
    const SimulationInputReader::Document document("input/spec/spec_multi.json");
    REQUIRE(document.isMultipleRocket());
    const auto readResult = document.toSimulationInput();
    const auto& input = readResult.value;

    REQUIRE(document.specificationName() == "spec_multi");
    REQUIRE(input.rocket.isMultiple());
    REQUIRE(input.rocket.bodyCount() == 3);
    REQUIRE(input.rocket.separations().size() == 1);
    REQUIRE(input.rocket.separations()[0].sourceBodyIndex == 0);
    REQUIRE(input.rocket.separations()[0].productBodyIndices == (std::vector<size_t>{1, 2}));
    REQUIRE(input.rocket.bodySpec(0).massInitial == Approx(8.243));
    REQUIRE(input.rocket.bodySpec(1).massInitial == Approx(5.589));
    REQUIRE(input.rocket.bodySpec(2).massInitial == Approx(2.278));
}

TEST_CASE("Aerodynamic CSV is converted to typed coefficient data", "[io][input]") {
    const SimulationInputReader::Document document("../test/data/spec_input/spec_aero.json");
    const auto readResult = document.toSimulationInput();
    const auto& input = readResult.value;
    const auto& storage = input.rocket.bodySpec(0).aeroCoefStorage;

    REQUIRE(storage.isTimeSeriesSpec());
    const auto aero = storage.valuesIn(10.0, 0.1, false);
    REQUIRE(aero.Cp == Approx(1.65));
    REQUIRE(aero.Cd == Approx(1.515));
    REQUIRE(aero.Cna == Approx(1.5));
}

TEST_CASE("Missing thrust data file is reported as an input error", "[io][input]") {
    const SimulationInputReader::Document document(
        "../test/data/spec_input/spec_missing_thrust.json");

    REQUIRE_THROWS_WITH(document.toSimulationInput(),
                        Catch::Contains("Failed to open thrust data file")
                            && Catch::Contains("missing-thrust-data.txt"));
}
