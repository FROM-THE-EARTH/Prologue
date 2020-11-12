#include "Quaternion.h"


Quaternion::Quaternion(double angle, double direction) {
	const double yaw = direction * Constant::PI / 180;
	const double pitch = -angle * Constant::PI / 180;
	const double roll = 0.0;

	const double
		cy = cos(yaw * 0.5),
		sy = sin(yaw * 0.5),
		cp = cos(pitch * 0.5),
		sp = sin(pitch * 0.5),
		cr = cos(roll * 0.5),
		sr = sin(roll * 0.5);


	x = cy * cp * sr - sy * sp * cr;
	y = sy * cp * sr + cy * sp * cr;
	z = sy * cp * cr - cy * sp * sr;
	w = cy * cp * cr + sy * sp * sr;
}


Quaternion Quaternion::angularVelocityApplied(const Vector3D& v) {
	return {
		0.5 * (y * v.z - z * v.y + w * v.x),
		0.5 * (-x * v.z + z * v.x + w * v.y),
		0.5 * (x * v.y - y * v.x + w * v.z),
		0.5 * (-x * v.x - y * v.y - z * v.z)

		//‚±‚Á‚¿‚ÌŽ®‚ª‘Ã“–‚È‹C‚ª‚µ‚Ä‚é https://arxiv.org/abs/0811.2889
		/*0.5 * (-v.z * y + v.y * z + v.x * w),
		0.5 * (v.z * x - v.x * z + v.y * w),
		0.5 * (-v.y * x + v.x * y + v.z * w),
		0.5 * (-v.x * x - v.y * y - v.z * z)*/
	};
}