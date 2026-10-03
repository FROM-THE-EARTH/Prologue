// ------------------------------------------------
// ResultDirectory.hppのテスト
// ------------------------------------------------

#define CATCH_CONFIG_MAIN

#include "cli/ResultDirectory.hpp"

#include <utility>
#include <vector>

#include "catch2/catch.hpp"

namespace {
    SimulationInput MakeInput(WindModelType windModel, TrajectoryMode trajectoryMode) {
        std::vector<BodySpecification> bodies(1);
        return {
            .environment = {
                .launchSite = {.name = "test",
                               .latitude = 35.0,
                               .longitude = 139.0,
                               .coordinateZone = 9,
                               .magneticDeclination = 0.0},
                .launchRail = {.length = 1.0, .azimuth = 0.0, .elevation = 90.0},
            },
            .rocket = RocketSpecification(std::move(bodies)),
            .solver = {.wind = {.type = windModel}},
            .run = {.trajectoryMode = trajectoryMode,
                    .windSpeed = 2.0,
                    .windDirection = {.degrees = 30.0}},
        };
    }
}

TEST_CASE("CLI result directory naming remains outside the simulation runner", "[cli][result]") {
    REQUIRE(cli::ResultDirectory::BuildName(
                "spec", SimulationMode::Detail,
                MakeInput(WindModelType::NoWind, TrajectoryMode::Trajectory), "unused.csv")
            == "spec[nowind_detail_traj]");

    REQUIRE(cli::ResultDirectory::BuildName(
                "spec", SimulationMode::Detail,
                MakeInput(WindModelType::Original, TrajectoryMode::Parachute), "unused.csv")
            == "spec[original_detail_para][2.00ms, 30.00deg]");

    REQUIRE(cli::ResultDirectory::BuildName(
                "spec", SimulationMode::Scatter,
                MakeInput(WindModelType::Original, TrajectoryMode::Trajectory), "unused.csv")
            == "spec[original_scatter_traj]");

    REQUIRE(cli::ResultDirectory::BuildName(
                "spec", SimulationMode::Detail,
                MakeInput(WindModelType::Real, TrajectoryMode::Trajectory), "sample.csv")
            == "spec[(sample)_traj]");

    const auto windFile = std::u8string(u8"\u6e2c\u5b9a \u98a8.csv");
    const auto expected = std::u8string(u8"spec[(\u6e2c\u5b9a \u98a8)_traj]");
    REQUIRE(cli::ResultDirectory::BuildName(
                "spec", SimulationMode::Detail,
                MakeInput(WindModelType::Real, TrajectoryMode::Trajectory),
                std::string(windFile.begin(), windFile.end()))
            == std::string(expected.begin(), expected.end()));
}
