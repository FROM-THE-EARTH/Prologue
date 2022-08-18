// ------------------------------------------------
// Vector3D.hppの実装
// ------------------------------------------------

#include "Vector3D.hpp"

#include "Quaternion.hpp"

Vector3D Vector3D::rotate(const Quaternion &q) const {
    if (length() == 0) {
        return Vector3D(0, 0, 0);
    } else {
        const Quaternion vector    = Quaternion(x, y, z, 0).normalized();
        const Quaternion newVector = q * vector * q.conjugated();
        return Vector3D(newVector.x, newVector.y, newVector.z) * length();
    }
}
