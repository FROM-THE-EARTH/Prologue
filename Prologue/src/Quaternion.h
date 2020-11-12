#pragma once
#include "Vector3D.h"
#include "Constant.h"


struct Quaternion {

	
	double x, y, z, w;


	constexpr Quaternion()
		:x(0.0),
		y(0.0),
		z(0.0),
		w(0.0)
	{}


	constexpr Quaternion(double _x, double _y, double _z, double _w)
		:x(_x),
		y(_y),
		z(_z),
		w(_w)
	{}


	Quaternion(double angle, double direction);


	/*========================method=============================*/

	//return the length of quaternion
	double length() {
		return sqrt(x * x + y * y + z * z + w * w);
	}

	//return the quaternion normalized
	Quaternion normalized() {
		return { x / length(),y / length() ,z / length() ,w / length() };
	}


	//to angle
	double toAngle() {
		Quaternion q(1, 0, 0, 0);
		q = *this * q * conjugate();
		return acos(sqrt(q.x * q.x + q.y * q.y) / sqrt(q.x * q.x + q.y * q.y + q.z * q.z)) * 180 / Constant::PI;
	}

	/*=======================method================================*/

	//return the quaternion angular velocity applied
	Quaternion angularVelocityApplied(const Vector3D& v);


	//return the conjugate quaternion
	Quaternion conjugate()const {
		return {
			-x, -y, -z, w
		};
	}


	/*============================operator=========================*/

	constexpr Quaternion operator*(const Quaternion& q)const  {
		return{
		w * q.x - z * q.y + y * q.z + x * q.w,
		z * q.x + w * q.y - x * q.z + y * q.w,
		-y * q.x + x * q.y + w * q.z + z * q.w,
		-x * q.x - y * q.y - z * q.z + w * q.w
		};
	}


	Quaternion& operator+=(const Quaternion& q) {
		x += q.x;
		y += q.y;
		z += q.z;
		w += q.w;

		return *this;
	}


	Quaternion& operator*=(const Quaternion& q) {
		x = w * q.x - z * q.y + y * q.z + x * q.w;
		y = z * q.x + w * q.y - x * q.z + y * q.w;
		z = -y * q.x + x * q.y + w * q.z + z * q.w;
		w = -x * q.x - y * q.y - z * q.z + w * q.w;

		return *this;
	}


	constexpr Quaternion operator*(double value)const {
		return { x * value, y * value, z * value,w * value };
	}

};



//standard output
template <class Char>
inline std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& os, const Quaternion& q)
{
	return os << Char('(') << q.x << Char(',') << q.y << Char(',') << q.z << Char(',') << q.w << Char(')');
}