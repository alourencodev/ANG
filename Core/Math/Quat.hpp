#pragma once

#include "Mat.hpp"
#include "SArray.hpp"
#include "Vec3.hpp"

union quat
{
public:
	struct
	{
		float w;
		float x, y, z;
	};
	struct
	{
		float angle;
		vec3 axis;
	};


	quat(float value = 0.0f) : angle(value), axis(vec3(value)) {}
	quat(float inW, float inX, float inY, float inZ) :
		w(inW), x(inX), y(inY), z(inZ) {}
	quat(float inAngle, const vec3 &inAxis) :
		angle(inAngle), axis(inAxis) {}

	void operator = (const quat &other)
	{
		w = other.w;
		x = other.x;
		y = other.y;
		z = other.z;
	}

	float operator [] (u32 index) const { return _elements[index]; }
	float &operator [] (u32 index) { return _elements[index]; }

	quat operator + (const quat &other) const
	{
		return quat(w + other.w, x + other.x, y + other.y, z + other.z);
	}

	quat operator - (const quat &other) const
	{
		return quat(w - other.w, x - other.x, y - other.y, z - other.z);
	}

	quat operator * (const quat &other) const
	{
		const float newT = (w * other.w) - (x * other.x) - (y * other.y) - (z * other.z);
		const float newX = (w * other.x) + (x * other.w) + (y * other.z) - (z * other.y);
		const float newY = (w * other.y) + (y * other.w) + (z * other.x) - (x * other.z);
		const float newZ = (w * other.z) + (z * other.w) + (x * other.y) - (y * other.x);

		return quat(newT, newX, newY, newZ);
	}

	template<typename t_scalarType>
	quat operator * (t_scalarType scalar) const
	{
		return quat(w * scalar, x * scalar, y * scalar, z * scalar);
	}

	template<typename t_scalarType>
	quat operator / (t_scalarType scalar) const
	{
		return quat(w / scalar, x / scalar, y / scalar, z / scalar);
	}

	bool operator == (const quat &other) const
	{
		return (w == other.w) && (x == other.x) && (y == other.y) && (z == other.z);
	}

	bool operator != (const quat &other) const
	{
		return (w != other.w) || (x != other.x) || (y != other.y) || (z != other.z);
	}

	friend quat normalize(const quat &q);

	operator mat4()
	{
		quat n = normalize(*this);

		float xx = n.x * n.x;
		float xy = n.x * n.y;
		float xz = n.x * n.z;
		float xw = n.x * n.w;

		float yy = n.y * n.y;
		float yz = n.y * n.z;
		float yw = n.y * n.w;

		float zz = n.z * n.z;
		float zw = n.z * n.w;

		return mat4({1 - 2 * (yy + zz),		2 * (xy + zw),	   2 * (xz - yw), 0.0f,
						 2 * (xy - zw),	1 - 2 * (xx + zz),	   2 * (yz + xw), 0.0f,
						 2 * (xz + yw),		2 * (yz - xw), 1 - 2 * (xx + yy), 0.0f,
					              0.0f,		         0.0f,				0.0f, 1.0f});
	}

private:
	SArray<float, 4> _elements;
};


template<typename t_scalarType>
static quat operator * (t_scalarType scalar, const quat &q)
{
	return q * scalar;
}

static float sqrLength(const quat &q)
{
	return (q.w * q.w) + (q.x * q.x) + (q.y * q.y) + (q.z * q.z);
}

static float length(const quat &q)
{
	return sqrtf(sqrLength(q));
}

static quat normalize(const quat &q)
{
	float len = length(q);

	if (len == 0.0f)
		return quat(0.0f);

	return q / len;
}

static std::ostream & operator << (std::ostream &os, const quat &q)
{
	os << "(" << q.w << ", (" << q.x << ", " << q.y << ", " << q.z << "))";
	return os;
}

static std::istream & operator >> (std::istream &is, quat &q)
{
	is >> q.w >> q.x >> q.y >> q.z;
	return is;
}
