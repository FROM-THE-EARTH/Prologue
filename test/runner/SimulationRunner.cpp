// ------------------------------------------------
// SimulationRunner.hppのテスト
// ------------------------------------------------

#define CATCH_CONFIG_MAIN

#include "runner/SimulationRunner.hpp"

#include <numbers>
#include <utility>
#include <vector>

#include "catch2/catch.hpp"

namespace {
    SimulationInput MakeInput() {
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
        body.aeroCoefStorage.init(0.7, 0.0, 0.3, 0.3, 0.0, 0.0);
        body.engine = Engine({{.time = 0.0, .thrust = 0.0},
                              {.time = 0.01, .thrust = 25.0},
                              {.time = 0.20, .thrust = 25.0},
                              {.time = 0.21, .thrust = 0.0}});

        std::vector<BodySpecification> bodies;
        bodies.emplace_back(std::move(body));
        return {
            .environment = {
                .launchSite = {.name = "test",
                               .latitude = 35.0,
                               .longitude = 139.0,
                               .coordinateZone = 9,
                               .magneticDeclination = 0.0},
                .launchRail = {.length = 0.5, .azimuth = 10.0, .elevation = 90.0},
            },
            .rocket = RocketSpecification(std::move(bodies)),
            .solver = {.timeStep = 0.01,
                       .resultStepSaveInterval = 1,
                       .atmosphere = {},
                       .wind = {.type = WindModelType::NoWind}},
            .run = {},
        };
    }

    class RecordingProgressObserver final : public SimulationProgressObserver {
    public:
        size_t total = 0;
        std::vector<size_t> completed;

        void onStarted(size_t totalSimulationCount) override {
            total = totalSimulationCount;
        }

        void onProgress(size_t completedSimulationCount, size_t totalSimulationCount) override {
            REQUIRE(totalSimulationCount == total);
            completed.push_back(completedSimulationCount);
        }
    };
}

TEST_CASE("Detail runner delegates to the core simulation", "[runner][detail]") {
    const auto result = SimulationRunner::RunDetail(MakeInput());

    REQUIRE(result.bodyResults.size() == 1);
    REQUIRE(result.bodyResults[0].steps.size() == 93);
    REQUIRE(result.maxAltitude == Approx(0.77403497751069505).margin(1e-12));
}

TEST_CASE("Scatter runner preserves wind-condition order and reports progress", "[runner][scatter]") {
    RecordingProgressObserver progress;
    const auto results = SimulationRunner::RunScatter(
        MakeInput(),
        {.windSpeedMin = 0.0, .windSpeedMax = 1.0, .windDirectionInterval = 180.0},
        {.multiThread = false, .threadCount = 1},
        &progress);

    REQUIRE(results.size() == 4);
    REQUIRE(results[0].windSpeed == 0.0);
    REQUIRE(results[0].windDirection.degrees == 10.0);
    REQUIRE(results[1].windSpeed == 0.0);
    REQUIRE(results[1].windDirection.degrees == 190.0);
    REQUIRE(results[2].windSpeed == 1.0);
    REQUIRE(results[2].windDirection.degrees == 10.0);
    REQUIRE(results[3].windSpeed == 1.0);
    REQUIRE(results[3].windDirection.degrees == 190.0);
    REQUIRE(results[0].bodyResults[0].steps.size() == 1);
    REQUIRE(progress.total == 4);
    REQUIRE(progress.completed == (std::vector<size_t>{1, 2, 3, 4}));
}

TEST_CASE("Parallel scatter execution keeps deterministic result ordering", "[runner][scatter]") {
    RecordingProgressObserver progress;
    const auto singleThreadResults = SimulationRunner::RunScatter(
        MakeInput(),
        {.windSpeedMin = 0.0, .windSpeedMax = 1.0, .windDirectionInterval = 180.0},
        {.multiThread = false, .threadCount = 1});
    const auto parallelResults = SimulationRunner::RunScatter(
        MakeInput(),
        {.windSpeedMin = 0.0, .windSpeedMax = 1.0, .windDirectionInterval = 180.0},
        {.multiThread = true, .threadCount = 2},
        &progress);

    REQUIRE(parallelResults.size() == singleThreadResults.size());
    REQUIRE(progress.completed == (std::vector<size_t>{1, 2, 3, 4}));
    for (size_t i = 0; i < parallelResults.size(); i++) {
        REQUIRE(parallelResults[i].windSpeed == singleThreadResults[i].windSpeed);
        REQUIRE(parallelResults[i].windDirection.degrees
                == singleThreadResults[i].windDirection.degrees);
        REQUIRE(parallelResults[i].bodyFinalPositions[0].x
                == singleThreadResults[i].bodyFinalPositions[0].x);
        REQUIRE(parallelResults[i].bodyFinalPositions[0].y
                == singleThreadResults[i].bodyFinalPositions[0].y);
        REQUIRE(parallelResults[i].bodyFinalPositions[0].z
                == singleThreadResults[i].bodyFinalPositions[0].z);
    }
}

TEST_CASE("Scatter runner rejects invalid execution settings", "[runner][validation]") {
    REQUIRE_THROWS_AS(
        SimulationRunner::RunScatter(
            MakeInput(),
            {.windSpeedMin = 0.0, .windSpeedMax = 1.0, .windDirectionInterval = 0.0},
            {}),
        std::invalid_argument);
    REQUIRE_THROWS_AS(
        SimulationRunner::RunScatter(
            MakeInput(),
            {.windSpeedMin = 0.0, .windSpeedMax = 1.0, .windDirectionInterval = 90.0},
            {.multiThread = true, .threadCount = 0}),
        std::invalid_argument);
}
