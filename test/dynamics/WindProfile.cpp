#define CATCH_CONFIG_MAIN

#include <limits>
#include <stdexcept>
#include <vector>

#include "catch2/catch.hpp"
#include "dynamics/WindProfile.hpp"

namespace {
    void RequireVector(const Vector3D& actual, const Vector3D& expected) {
        REQUIRE(actual.x == Approx(expected.x).margin(1e-10));
        REQUIRE(actual.y == Approx(expected.y).margin(1e-10));
        REQUIRE(actual.z == Approx(expected.z).margin(1e-10));
    }
}

TEST_CASE("Wind profile interpolates from zero wind at ground", "[dynamics][wind]") {
    const WindProfile profile(
        {WindData{.geopotentialHeight = 100.0, .speed = 4.0, .direction = 90.0},
         WindData{.geopotentialHeight = 200.0, .speed = 6.0, .direction = 90.0}});

    RequireVector(profile.windAt(-1.0), Vector3D());
    RequireVector(profile.windAt(0.0), Vector3D());
    RequireVector(profile.windAt(50.0), Vector3D(-2.0, 0.0, 0.0));
    RequireVector(profile.windAt(100.0), Vector3D(-4.0, 0.0, 0.0));
}

TEST_CASE("Wind profile accepts an explicit zero-wind ground observation", "[dynamics][wind]") {
    const WindProfile profile(
        {WindData{.geopotentialHeight = 0.0, .speed = 0.0, .direction = 45.0},
         WindData{.geopotentialHeight = 100.0, .speed = 4.0, .direction = 90.0}});

    RequireVector(profile.windAt(0.0), Vector3D());
    RequireVector(profile.windAt(50.0), Vector3D(-2.0, 0.0, 0.0));
}

TEST_CASE("Wind profile reproduces observations and rejects heights above its range", "[dynamics][wind]") {
    const WindProfile profile(
        {WindData{.geopotentialHeight = 100.0, .speed = 4.0, .direction = 0.0},
         WindData{.geopotentialHeight = 200.0, .speed = 6.0, .direction = 90.0}});

    RequireVector(profile.windAt(100.0), Vector3D(0.0, -4.0, 0.0));
    RequireVector(profile.windAt(200.0), Vector3D(-6.0, 0.0, 0.0));
    REQUIRE_THROWS_AS(profile.windAt(1000.0), std::out_of_range);
}

TEST_CASE("Wind profile interpolates vectors across the north direction", "[dynamics][wind]") {
    const WindProfile profile(
        {WindData{.geopotentialHeight = 100.0, .speed = 10.0, .direction = 350.0},
         WindData{.geopotentialHeight = 200.0, .speed = 10.0, .direction = 10.0}});

    const Vector3D middle = profile.windAt(150.0);
    REQUIRE(middle.x == Approx(0.0).margin(1e-10));
    REQUIRE(middle.y < 0.0);
    REQUIRE(middle.z == Approx(0.0).margin(1e-10));
}

TEST_CASE("Wind profile rejects invalid observations", "[dynamics][wind]") {
    REQUIRE_THROWS_AS(WindProfile(std::vector<WindData>()), std::invalid_argument);
    REQUIRE_THROWS_AS(WindProfile({WindData{.geopotentialHeight = 0.0, .speed = 1.0, .direction = 0.0}}),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(WindProfile({WindData{.geopotentialHeight = 100.0, .speed = -1.0, .direction = 0.0}}),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(WindProfile({WindData{.geopotentialHeight = 200.0, .speed = 1.0, .direction = 0.0},
                                   WindData{.geopotentialHeight = 100.0, .speed = 1.0, .direction = 0.0}}),
                      std::invalid_argument);
    REQUIRE_THROWS_AS(WindProfile({WindData{.geopotentialHeight = 100.0,
                                            .speed = std::numeric_limits<double>::quiet_NaN(),
                                            .direction = 0.0}}),
                      std::invalid_argument);
}
