#define CATCH_CONFIG_MAIN

#include "core/Simulation.hpp"

#include <numbers>
#include <limits>
#include <utility>
#include <vector>

#include "catch2/catch.hpp"

namespace {
    BodySpecification MakeBody(bool powered) {
        BodySpecification body;
        body.length = 1.0;
        body.diameter = 0.1;
        body.bottomArea = body.diameter * body.diameter * 0.25 * std::numbers::pi;
        body.CGLengthInitial = 0.5;
        body.CGLengthFinal = 0.5;
        body.massInitial = 1.0;
        body.massFinal = 1.0;
        body.rollingMomentInertiaInitial = 0.1;
        body.rollingMomentInertiaFinal = 0.1;
        body.Cmq = 0.0;
        body.aeroCoefStorage.init(0.7, 0.0, 0.3, 0.3, 0.0, 0.0);
        if (powered) {
            body.engine = Engine({{.time = 0.0, .thrust = 0.0},
                                  {.time = 0.01, .thrust = 25.0},
                                  {.time = 0.20, .thrust = 25.0},
                                  {.time = 0.21, .thrust = 0.0}});
        }
        return body;
    }

    SimulationInput MakeSingleInput() {
        std::vector<BodySpecification> bodies;
        bodies.emplace_back(MakeBody(true));
        return {
            .environment = {
                .launchSite = {.name = "test",
                               .latitude = 35.0,
                               .longitude = 139.0,
                               .coordinateZone = 9,
                               .magneticDeclination = 0.0},
                .launchRail = {.length = 0.5, .azimuth = 0.0, .elevation = 90.0},
            },
            .rocket = RocketSpecification(std::move(bodies)),
            .solver = {.timeStep = 0.01,
                       .resultStepSaveInterval = 1,
                       .atmosphere = {},
                       .wind = {.type = WindModelType::NoWind}},
            .run = {},
        };
    }

    SimulationInput MakeLegacyMultiInput() {
        std::vector<BodySpecification> bodies;
        bodies.emplace_back(MakeBody(true));
        bodies.emplace_back(MakeBody(false));
        bodies.emplace_back(MakeBody(false));
        std::vector<SeparationSpecification> separations{
            {.sourceBodyIndex = 0, .productBodyIndices = {1, 2}},
        };
        return {
            .environment = {
                .launchSite = {.name = "test",
                               .latitude = 35.0,
                               .longitude = 139.0,
                               .coordinateZone = 9,
                               .magneticDeclination = 0.0},
                .launchRail = {.length = 0.5, .azimuth = 0.0, .elevation = 90.0},
            },
            .rocket = RocketSpecification(std::move(bodies), std::move(separations)),
            .solver = {.timeStep = 0.01,
                       .resultStepSaveInterval = 1,
                       .atmosphere = {},
                       .wind = {.type = WindModelType::NoWind}},
            .run = {.trajectoryMode = TrajectoryMode::Trajectory,
                    .detachType = DetachType::Time,
                    .detachTime = 0.30},
        };
    }
}

TEST_CASE("Core simulation facade returns a deterministic single-body result", "[core][solver][regression]") {
    const auto result = Simulation::Run(MakeSingleInput());

    REQUIRE(result.bodyResults.size() == 1);
    REQUIRE(result.bodyFinalPositions.size() == 1);
    REQUIRE(result.windDirection.reference == DirectionReference::TrueNorth);
    REQUIRE(result.bodyResults[0].steps.size() == 93);
    REQUIRE(result.bodyResults[0].steps.back().rocket_pos.z == 0.0);
    REQUIRE(result.bodyFinalPositions[0].z == Approx(-0.031905770820604175).margin(1e-12));
    REQUIRE(result.maxAltitude == Approx(0.77403497751069505).margin(1e-12));
    REQUIRE(result.maxDynamicPressureDuringRising == Approx(5.6523062701395981).margin(1e-12));
}

TEST_CASE("Core simulation facade preserves the legacy three-body separation", "[core][solver][regression]") {
    const auto result = Simulation::Run(MakeLegacyMultiInput());

    REQUIRE(result.bodyResults.size() == 3);
    REQUIRE(result.bodyFinalPositions.size() == 3);
    REQUIRE(result.bodyResults[0].steps.size() == 30);
    REQUIRE(result.bodyResults[1].steps.size() == 63);
    REQUIRE(result.bodyResults[2].steps.size() == 63);
    REQUIRE(result.bodyFinalPositions[0].z == Approx(0.52669873605275219).margin(1e-12));
    REQUIRE(result.bodyFinalPositions[1].z == Approx(-0.031905770820604175).margin(1e-12));
    REQUIRE(result.bodyFinalPositions[2].z == Approx(-0.031905770820604175).margin(1e-12));
    REQUIRE(result.bodyResults[1].steps.back().rocket_pos.z == 0.0);
    REQUIRE(result.bodyResults[2].steps.back().rocket_pos.z == 0.0);
    REQUIRE(result.maxAltitude == Approx(0.77403497751069505).margin(1e-12));
}

TEST_CASE("Core rejects invalid solver settings before integration", "[core][validation]") {
    auto zeroInterval = MakeSingleInput();
    zeroInterval.solver.resultStepSaveInterval = 0;
    REQUIRE_THROWS_AS(Simulation::Run(zeroInterval), std::invalid_argument);

    auto zeroStep = MakeSingleInput();
    zeroStep.solver.timeStep = 0.0;
    REQUIRE_THROWS_AS(Simulation::Run(zeroStep), std::invalid_argument);

    auto missingMeasuredProfile = MakeSingleInput();
    missingMeasuredProfile.solver.wind.type = WindModelType::Real;
    REQUIRE_THROWS_AS(Simulation::Run(missingMeasuredProfile), std::invalid_argument);

    auto invalidPowerLaw = MakeSingleInput();
    invalidPowerLaw.solver.wind.type = WindModelType::OnlyPowerLow;
    invalidPowerLaw.solver.wind.powerConstant = 0.0;
    REQUIRE_THROWS_AS(Simulation::Run(invalidPowerLaw), std::invalid_argument);

    auto invalidDirection = MakeSingleInput();
    invalidDirection.run.windDirection.degrees = std::numeric_limits<double>::quiet_NaN();
    REQUIRE_THROWS_AS(Simulation::Run(invalidDirection), std::invalid_argument);

    auto invalidLaunchSite = MakeSingleInput();
    invalidLaunchSite.environment.launchSite.coordinateZone = 0;
    REQUIRE_THROWS_AS(Simulation::Run(invalidLaunchSite), std::invalid_argument);
}

TEST_CASE("Core rejects unsupported separation topology instead of indexing implicitly", "[core][validation]") {
    auto input = MakeSingleInput();
    std::vector<BodySpecification> bodies;
    bodies.emplace_back(MakeBody(true));
    bodies.emplace_back(MakeBody(false));
    input.rocket = RocketSpecification(
        std::move(bodies),
        {{.sourceBodyIndex = 0, .productBodyIndices = {1}}});

    REQUIRE_THROWS_AS(Simulation::Run(input), std::invalid_argument);
}

TEST_CASE("Landing-point reduction handles a body detached before recording", "[core][regression]") {
    auto input = MakeLegacyMultiInput();
    input.run.detachTime = 0.0;
    const auto detail = Simulation::Run(input);
    REQUIRE(detail.bodyResults.size() == 3);
    REQUIRE(detail.bodyResults[0].steps.empty());

    const auto landing = Simulation::LandingPointsOnly(detail);
    REQUIRE(landing.bodyResults.size() == 2);
    REQUIRE(landing.bodyFinalPositions == detail.bodyFinalPositions);
    for (const auto& body : landing.bodyResults) {
        REQUIRE(body.steps.size() == 1);
        REQUIRE(body.steps.front().rocket_pos.z == 0.0);
    }

    REQUIRE(Simulation::LandingPointsOnly({}).bodyResults.empty());
}
