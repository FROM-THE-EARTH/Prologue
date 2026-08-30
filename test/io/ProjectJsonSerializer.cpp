// ------------------------------------------------
// ProjectJsonSerializer.hppのテスト
// ------------------------------------------------

#define CATCH_CONFIG_MAIN

#include "io/FileProjectResourceProvider.hpp"
#include "io/ProjectJsonSerializer.hpp"
#include "project/SimulationInputBuilder.hpp"

#include <filesystem>
#include <vector>

#include "catch2/catch.hpp"

namespace {
    SimulationInput LoadSimulationInput(const std::filesystem::path& projectFile) {
        const auto document = ProjectJsonSerializer::Load(projectFile);
        const ProjectIO::FileProjectResourceProvider resources(projectFile.parent_path());
        return project::BuildSimulationInput(document, resources);
    }
}

TEST_CASE("Single-body project is converted to typed input", "[io][project]") {
    const std::filesystem::path file = "input/spec/spec_single.json";
    const auto document = ProjectJsonSerializer::Load(file);
    const auto input = LoadSimulationInput(file);

    REQUIRE(document.formatVersion == project::CurrentFormatVersion);
    REQUIRE(document.bodies.size() == 1);
    REQUIRE(document.separations.empty());
    REQUIRE(document.bodies[0].engine.thrustFile
            == std::filesystem::path("../thrust/Sample_K240.txt"));
    REQUIRE(input.environment.launchSite.name == "nosiro_sea");
    REQUIRE(input.environment.launchSite.latitude == Approx(40.242865));
    REQUIRE(input.environment.launchSite.longitude == Approx(140.01045));
    REQUIRE(input.environment.launchSite.coordinateZone == 10);
    REQUIRE(input.environment.launchSite.magneticDeclination == Approx(8.94));
    REQUIRE(input.environment.launchRail.length == Approx(5.0));
    REQUIRE(input.environment.launchRail.azimuth == Approx(-60.0));
    REQUIRE(input.environment.launchRail.elevation == Approx(70.0));

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

TEST_CASE("Explicit separation topology is preserved", "[io][project]") {
    const auto input = LoadSimulationInput("input/spec/spec_multi.json");

    REQUIRE(input.rocket.isMultiple());
    REQUIRE(input.rocket.bodyCount() == 3);
    REQUIRE(input.rocket.separations().size() == 1);
    REQUIRE(input.rocket.separations()[0].sourceBodyIndex == 0);
    REQUIRE(input.rocket.separations()[0].productBodyIndices == (std::vector<size_t>{1, 2}));
    REQUIRE(input.rocket.bodySpec(0).massInitial == Approx(8.243));
    REQUIRE(input.rocket.bodySpec(1).massInitial == Approx(5.589));
    REQUIRE(input.rocket.bodySpec(2).massInitial == Approx(2.278));
}

TEST_CASE("Aerodynamic CSV reference is resolved relative to the project", "[io][project]") {
    const auto input = LoadSimulationInput("../test/data/spec_input/spec_aero.json");
    const auto& storage = input.rocket.bodySpec(0).aeroCoefStorage;

    REQUIRE(storage.isTimeSeriesSpec());
    const auto aero = storage.valuesIn(10.0, 0.1, false);
    REQUIRE(aero.Cp == Approx(1.65));
    REQUIRE(aero.Cd == Approx(1.515));
    REQUIRE(aero.Cna == Approx(1.5));
}

TEST_CASE("Missing thrust data file is reported as an input error", "[io][project]") {
    REQUIRE_THROWS_WITH(LoadSimulationInput("../test/data/spec_input/spec_missing_thrust.json"),
                        Catch::Contains("Failed to open thrust data file")
                            && Catch::Contains("missing-thrust-data.txt"));
}

TEST_CASE("Project JSON can be saved and loaded without losing authored values", "[io][project]") {
    const auto source = ProjectJsonSerializer::Load("input/spec/spec_multi.json");
    const auto destination = std::filesystem::temp_directory_path() / "prologue-project-round-trip.json";

    ProjectJsonSerializer::Save(destination, source);
    const auto restored = ProjectJsonSerializer::Load(destination);
    std::filesystem::remove(destination);

    REQUIRE(restored.formatVersion == source.formatVersion);
    REQUIRE(restored.environment.place == source.environment.place);
    REQUIRE(restored.environment.latitude == source.environment.latitude);
    REQUIRE(restored.environment.longitude == source.environment.longitude);
    REQUIRE(restored.environment.coordinateZone == source.environment.coordinateZone);
    REQUIRE(restored.environment.magneticDeclination
            == source.environment.magneticDeclination);
    REQUIRE(restored.environment.railLength == source.environment.railLength);
    REQUIRE(restored.bodies.size() == source.bodies.size());
    REQUIRE(restored.bodies[0].length == source.bodies[0].length);
    REQUIRE(restored.bodies[0].engine.thrustFile == source.bodies[0].engine.thrustFile);
    REQUIRE(restored.bodies[1].parachutes[0].openingHeight
            == source.bodies[1].parachutes[0].openingHeight);
    REQUIRE(restored.separations[0].sourceBodyIndex == source.separations[0].sourceBodyIndex);
    REQUIRE(restored.separations[0].productBodyIndices
            == source.separations[0].productBodyIndices);
}
