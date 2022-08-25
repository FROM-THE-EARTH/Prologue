#define CATCH_CONFIG_MAIN

#include "math/Quaternion.hpp"

#include "catch2/catch.hpp"

TEST_CASE("Quaternion", "[math]") {
    SECTION("Self") {
        const Quaternion q(1, 2, 3, 4);
        REQUIRE(q.length() == sqrt(1 * 1 + 2 * 2 + 3 * 3 + 4 * 4));
        REQUIRE(q.normalized().length() == Approx(1));
        REQUIRE(q.conjugated() == Quaternion(-1, -2, -3, 4));
    }
    SECTION("Operators") {
        SECTION("With scalar") {
            Quaternion q(1, 2, 3, 4);
            REQUIRE(q * 2 == Quaternion(2, 4, 6, 8));
            REQUIRE(q / 2 == Quaternion(0.5, 1, 1.5, 2));
            q *= 2;
            REQUIRE(q == Quaternion(2, 4, 6, 8));
            q /= 2;
            REQUIRE(q == Quaternion(1, 2, 3, 4));
        }
        SECTION("With quaternion") {
            Quaternion q1(1, 2, 3, 4);
            REQUIRE(q1 + q1 == Quaternion(2, 4, 6, 8));
            q1 += q1;
            REQUIRE(q1 == Quaternion(2, 4, 6, 8));

            Quaternion q2(90, 0);
            REQUIRE((q2 * q2).toAngle() == Approx(180));
            q2 *= q2;
            REQUIRE(q2.toAngle() == Approx(180));
        }
    }
    SECTION("Initialize with angle and direction") {
        REQUIRE(Quaternion(90, 0).toAngle() == Approx(90));
        REQUIRE(Quaternion(0, 90).toAngle() == Approx(90));
        REQUIRE(Quaternion(90, 90).toAngle() == Approx(90));
        REQUIRE(Quaternion(45, 45).toAngle() == Approx(60));
    }
    SECTION("Rotation") {
        REQUIRE((Quaternion(90, 0) * Quaternion(90, 0)).toAngle() == Approx(180));
        REQUIRE((Quaternion(45, 0) * Quaternion(0, 45)).toAngle() == Approx(60));
        REQUIRE((Quaternion(90, 0) * Quaternion(90, 0) * Quaternion(90, 0) * Quaternion(90, 0)).toAngle() == Approx(0));
    }
    SECTION("Apply angular velocity") {
        const Quaternion q(1, 2, 3, 4);
        const Vector3D v(1, 2, 3);
        REQUIRE(q * Quaternion(v.x, v.y, v.z, 0) * 0.5 == q.angularVelocityApplied(v));
    }
    SECTION("Vector3D::rotated()") {
        const double margin = 1e-15;
        const Vector3D v1   = Vector3D(1, 0, 0).rotated(Quaternion(90, 0));
        REQUIRE(v1.x == Approx(0).margin(margin));
        REQUIRE(v1.y == Approx(0).margin(margin));
        REQUIRE(v1.z == Approx(1).margin(margin));
        const Vector3D v2 = Vector3D(1, 0, 0).rotated(Quaternion(180, 0));
        REQUIRE(v2.x == Approx(-1).margin(margin));
        REQUIRE(v2.y == Approx(0).margin(margin));
        REQUIRE(v2.z == Approx(0).margin(margin));
        const Vector3D v3 = Vector3D(1, 0, 0).rotated(Quaternion(45, 45));
        REQUIRE(v3.x == Approx(0.5).margin(margin));
        REQUIRE(v3.y == Approx(0.5).margin(margin));
        REQUIRE(v3.z == Approx(1 / sqrt(2)).margin(margin));
    }
}
