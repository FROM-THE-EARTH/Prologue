#define CATCH_CONFIG_MAIN

#include "math/Vector3D.hpp"

#include "catch2/catch.hpp"

// Vector3D::rotated() is tested in Quaternion.cpp

TEST_CASE("Vector3D", "[math]") {
    SECTION("Self") {
        Vector3D v(1, 2, 3);
        REQUIRE(v.length() == sqrt(1 * 1 + 2 * 2 + 3 * 3));
        REQUIRE(v.normalized().length() == Approx(1));
    }
    SECTION("Inner / Outer product") {
        const Vector3D v1(1, 2, 3), v2(3, 2, 1);
        REQUIRE((v1 ^ v2) == 10);
        REQUIRE((v1 | v2) == Vector3D(-4, 8, -4));
    }
    SECTION("Operators") {
        SECTION("With scalar") {
            Vector3D v(1, 2, 3);
            REQUIRE(v * 2 == Vector3D(2, 4, 6));
            REQUIRE(v / 2 == Vector3D(0.5, 1, 1.5));
            v *= 2;
            REQUIRE(v == Vector3D(2, 4, 6));
            v /= 2;
            REQUIRE(v == Vector3D(1, 2, 3));
        }
        SECTION("With vector") {
            Vector3D v1(1, 2, 3), v2(3, 2, 1);
            REQUIRE(v1 + v2 == Vector3D(4, 4, 4));
            REQUIRE(v1 - v2 == Vector3D(-2, 0, 2));
            v1 += v2;
            REQUIRE(v1 == Vector3D(4, 4, 4));
            v1 -= v2;
            REQUIRE(v1 == Vector3D(1, 2, 3));
        }
    }
}
