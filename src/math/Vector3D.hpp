// ------------------------------------------------
// 三次元ベクトルクラス
// ほとんどの関数は高速化のためインライン関数としてhppで定義
// ------------------------------------------------

#pragma once

#include <cmath>
#include <iostream>

struct Quaternion;

struct Vector3D {
    double x, y, z;

    constexpr Vector3D() : x(0.0), y(0.0), z(0.0) {}

    constexpr Vector3D(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}

    /*=======================method===========================*/
    double length() const {
        return std::sqrt(*this ^ *this);
    }

    Vector3D normalized() const {
        return *this / length();
    }

    Vector3D rotated(const Quaternion& q) const;

    /*======================operator==========================*/
    constexpr bool operator==(const Vector3D& v) const {
        return (x == v.x) && (y == v.y) && (z == v.z);
    }

    constexpr Vector3D operator+() const {
        return *this;
    }

    constexpr Vector3D operator-() const {
        return {-x, -y, -z};
    }

    constexpr Vector3D operator+(const Vector3D& v) const {
        return {x + v.x, y + v.y, z + v.z};
    }

    constexpr Vector3D operator-(const Vector3D& v) const {
        return {x - v.x, y - v.y, z - v.z};
    }

    constexpr Vector3D operator*(double value) const {
        return {x * value, y * value, z * value};
    }

    constexpr Vector3D operator/(double value) const {
        return {x / value, y / value, z / value};
    }

    constexpr Vector3D& operator+=(const Vector3D& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    constexpr Vector3D& operator-=(const Vector3D& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }

    constexpr Vector3D operator*=(double value) {
        x *= value;
        y *= value;
        z *= value;
        return *this;
    }

    constexpr Vector3D operator/=(double value) {
        x /= value;
        y /= value;
        z /= value;
        return *this;
    }

    // inner product
    constexpr double operator^(const Vector3D& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    // outer product
    constexpr Vector3D operator|(const Vector3D& v) const {
        return {y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x};
    }
};

inline constexpr Vector3D operator*(double value, const Vector3D& v) {
    return {value * v.x, value * v.y, value * v.z};
}

// standard output
template <class Char>
inline std::basic_ostream<Char>& operator<<(std::basic_ostream<Char>& os, const Vector3D& v) {
    return os << Char('(') << v.x << Char(',') << v.y << Char(',') << v.z << Char(')');
}

// standard input
template <class Char>
inline std::basic_istream<Char>& operator>>(std::basic_istream<Char>& is, Vector3D& v) {
    Char unused;
    return is >> unused >> v.x >> unused >> v.y >> unused >> v.z >> unused;
}
