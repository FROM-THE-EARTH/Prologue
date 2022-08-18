// ------------------------------------------------
// クォータニオンクラス
// ほとんどの関数は高速化のためインライン関数としてhppで定義
// ------------------------------------------------

#pragma once

#include "Vector3D.hpp"
#include "misc/Constant.hpp"

struct Quaternion {
    double x, y, z, w;

    constexpr Quaternion() : x(0.0), y(0.0), z(0.0), w(0.0) {}

    constexpr Quaternion(double _x, double _y, double _z, double _w) : x(_x), y(_y), z(_z), w(_w) {}

    explicit Quaternion(double angle, double direction);

    /*========================method=============================*/

    // return the length of quaternion
    double length() const {
        return sqrt(x * x + y * y + z * z + w * w);
    }

    // return the quaternion normalized
    Quaternion normalized() const {
        return Quaternion{x, y, z, w} / length();
    }

    // to angle
    double toAngle() const {
        Vector3D v1(1, 0, 0);
        Vector3D v2 = v1.rotate(*this);
        return acos((v1 ^ v2) / (v1.length() * v2.length())) * 180.0 / Constant::PI;
    }

    /*=======================method================================*/

    // return the quaternion angular velocity applied
    Quaternion angularVelocityApplied(const Vector3D& v) const;

    // return the conjugate quaternion
    Quaternion conjugated() const {
        return {-x, -y, -z, w};
    }

    /*============================operator=========================*/
    constexpr bool operator==(const Quaternion& q) const {
        return (x == q.x) && (y == q.y) && (z == q.z) && (w == q.w);
    }

    constexpr Quaternion operator+(const Quaternion& q) const {
        return {x + q.x, y + q.y, z + q.z, w + q.w};
    }

    constexpr Quaternion operator*(const Quaternion& q) const {
        return {w * q.x - z * q.y + y * q.z + x * q.w,
                z * q.x + w * q.y - x * q.z + y * q.w,
                -y * q.x + x * q.y + w * q.z + z * q.w,
                -x * q.x - y * q.y - z * q.z + w * q.w};
    }

    constexpr Quaternion operator*(double value) const {
        return {x * value, y * value, z * value, w * value};
    }

    constexpr Quaternion operator/(double value) const {
        return {x / value, y / value, z / value, w / value};
    }

    constexpr Quaternion& operator+=(const Quaternion& q) {
        *this = *this + q;
        return *this;
    }

    constexpr Quaternion& operator*=(const Quaternion& q) {
        *this = *this * q;
        return *this;
    }

    constexpr Quaternion operator*=(double value) {
        *this = *this * value;
        return *this;
    }

    constexpr Quaternion operator/=(double value) {
        *this = *this / value;
        return *this;
    }
};

// standard output
template <class Char>
inline std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& os, const Quaternion& q) {
    return os << Char('(') << q.x << Char(',') << q.y << Char(',') << q.z << Char(',') << q.w << Char(')');
}
