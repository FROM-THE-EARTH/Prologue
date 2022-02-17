#include "Vector3D.hpp"

#include "Quaternion.hpp"

Vector3D Vector3D::applyQuaternion(const Quaternion& q) {
    if (length() == 0) {
        return Vector3D(0, 0, 0);
    } else {
        const Quaternion vector    = Quaternion(x, y, z, 0).normalized();
        const Quaternion newVector = q * vector * q.conjugate();
        return Vector3D(newVector.x, newVector.y, newVector.z) * length();
    }
}
