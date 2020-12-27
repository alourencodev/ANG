#pragma once

#include <cmath>
#include <iostream>

#include "../Types.hpp"
#include "Vec2.hpp"

template<typename t_type>
union vector3
{
	ASSERT_IS_ARITHMETIC(t_type);

public:
	union
	{
		struct {t_type x, y, z;};
		struct {t_type r, g, b;};
	};

	vector3(t_type inX, t_type inY, t_type inZ) : x(inX), y(inY), z(inZ) {}
	vector3(const vector2<t_type> &vec, t_type inZ = static_cast<t_type>(0)) : vector3(vec.x, vec.y, inZ) {}
	vector3(t_type value) : vector3(value, value, value) {}
	vector3() :	vector3(static_cast<t_type>(0)) {}

	explicit operator vector2<t_type>() const 
	{ 
		return vector2<t_type>(x, y); 
	}

	void operator = (const vector3<t_type> &vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
	}

	t_type operator [] (int index) const
	{
		return reinterpret_cast<const t_type *>(this)[index];
	}

	t_type & operator [] (int index)
	{
		return reinterpret_cast<t_type *>(this)[index];
	}

	template<typename t_rightType>
	auto operator + (const vector3<t_rightType> &vec) const
	{
		ASSERT_IS_ARITHMETIC(t_rightType);
		using resultType = decltype(x + vec.x);

		return vector3<resultType>(x + vec.x, y + vec.y, z + vec.z);
	}

	template<typename t_rightType>
	auto operator - (const vector3<t_rightType> &vec) const
	{
		ASSERT_IS_ARITHMETIC(t_rightType);
		using resultType = decltype(x - vec.x);

		return vector3<resultType>(x - vec.x, y - vec.y, z - vec.z);
	}

	vector3<t_type> operator - () const
	{
		return vector3<t_type>(-x, -y, -z);
	}

	template<typename t_scalarType>
	auto operator * (t_scalarType value) const
	{
		ASSERT_IS_ARITHMETIC(t_scalarType);
		using resultType = decltype(x * value);

		return vector3<resultType>(x * value, y * value, z * value);
	}

	template<typename t_scalarType>
	vector3<float> operator / (t_scalarType value) const
	{
		ASSERT_IS_ARITHMETIC(t_scalarType);
		const float divisor = static_cast<float>(value);

		return vector3<float>(x / divisor, y / divisor, z / divisor);
	}

	bool operator == (const vector3<t_type> &vec) const
	{
		return (x == vec.x) && (y == vec.y) && (z == vec.z);
	}
};

template<typename t_vectorType, typename t_scalarType>
static auto operator * (t_scalarType scalar, const vector3<t_vectorType> &vec)
{
	ASSERT_IS_ARITHMETIC(t_scalarType);
	return vec * scalar;
}

template<typename t_type>
static bool operator != (const vector3<t_type> vecA, const vector3<t_type> &vecB)
{
	return !(vecA == vecB);
}

template<typename t_type>
static std::ostream & operator << (std::ostream &os, const vector3<t_type> &vec)
{
	os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	return os;
}

template <typename t_type>
static std::istream & operator >> (std::istream &is, vector3<t_type> &vec)
{
	is >> vec.x >> vec.y >> vec.z;
	return is;
}

template<typename t_type>
static float sqrLength(const vector3<t_type> &vec)
{
	return static_cast<float>((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
}

template<typename t_type>
static float length(const vector3<t_type> &vec)
{
	return sqrtf(sqrLength(vec));
}

template<typename t_type>
static t_type dot(const vector3<t_type> &vecA, const vector3<t_type> &vecB)
{
	return (vecA.x * vecB.x) + (vecA.y * vecB.y) + (vecA.z * vecB.z);
}

template<typename t_type>
static vector3<t_type> cross(const vector3<t_type> &vecA, const vector3<t_type> &vecB)
{
	vector3<t_type> result;
	result.x = (vecA.y * vecB.z) - (vecA.z * vecB.y); 
	result.y = (vecA.z * vecB.x) - (vecA.x * vecB.z);
	result.z = (vecA.x * vecB.y) - (vecA.y * vecB.x);

	return result;
}

template<typename t_type>
static vector3<float> normalize(const vector3<t_type> &vec)
{
	float len = length(vec);
	len = (len == 0) ? 1 : len;
	
	return static_cast<vector3<float>>(vec) / len;
}

using vec3	= vector3<float>;
using vec3i = vector3<i32>;

using color3 = vector3<float>;
using color3i = vector3<ui8>;

