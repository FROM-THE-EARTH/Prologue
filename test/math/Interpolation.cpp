// ------------------------------------------------
// Interpolation.hppのテスト
// ------------------------------------------------

#define CATCH_CONFIG_MAIN

#include "math/Interpolation.hpp"

#include "catch2/catch.hpp"

TEST_CASE("Linear interpolation preserves endpoint and interior values", "[math][interpolation]") {
    REQUIRE(Interpolation::Linear(0, 0, 1, 2, 3) == 2);
    REQUIRE(Interpolation::Linear(1, 0, 1, 2, 3) == 3);
    REQUIRE(Interpolation::Linear(1.5, 1, 2, 2, 3) == 2.5);
    REQUIRE(Interpolation::Linear(0, -1, 0, -2, -3) == -3);
    REQUIRE(Interpolation::Linear(-1, -1, 0, -2, -3) == -2);
    REQUIRE(Interpolation::Linear(-1.5, -2, -1, -2, -3) == -2.5);
}

TEST_CASE("Linear interpolation preserves the zero-width fallback", "[math][interpolation]") {
    REQUIRE(Interpolation::Linear(1.0, 2.0, 2.0, 3.0, 4.0) == 3.0);
}
