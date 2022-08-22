// ------------------------------------------------
// Quaternion.hppの実装
// ------------------------------------------------

#include "Quaternion.hpp"

Quaternion::Quaternion(double angle, double direction) {
    const double yaw   = direction * Constant::PI / 180;
    const double pitch = -angle * Constant::PI / 180;
    const double roll  = 0.0;

    const double cy = cos(yaw * 0.5), sy = sin(yaw * 0.5), cp = cos(pitch * 0.5), sp = sin(pitch * 0.5),
                 cr = cos(roll * 0.5), sr = sin(roll * 0.5);

    x = cy * cp * sr - sy * sp * cr;
    y = sy * cp * sr + cy * sp * cr;
    z = sy * cp * cr - cy * sp * sr;
    w = cy * cp * cr + sy * sp * sr;
}

Quaternion Quaternion::angularVelocityApplied(const Vector3D& v) const {
    return *this * Quaternion(v.x, v.y, v.z, 0) * 0.5;
}
