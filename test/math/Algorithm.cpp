#define CATCH_CONFIG_MAIN

#include "math/Algorithm.hpp"

#include "catch2/catch.hpp"

TEST_CASE("Algorithm", "[math]") {
    REQUIRE(Algorithm::Lerp(0, 0, 1, 2, 3) == 2);
    REQUIRE(Algorithm::Lerp(1, 0, 1, 2, 3) == 3);
    REQUIRE(Algorithm::Lerp(1.5, 1, 2, 2, 3) == 2.5);
    REQUIRE(Algorithm::Lerp(0, -1, 0, -2, -3) == -3);
    REQUIRE(Algorithm::Lerp(-1, -1, 0, -2, -3) == -2);
    REQUIRE(Algorithm::Lerp(-1.5, -2, -1, -2, -3) == -2.5);
}
