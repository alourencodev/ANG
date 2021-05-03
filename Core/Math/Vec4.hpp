#pragma once

#include <cmath>
#include <iostream>

#include "Log/Assert.hpp"
#include "Types.hpp"
#include "Vec2.hpp"
#include "Vec3.hpp"

template<typename t_type>
union vector4
{
	g_assertIsArithmetic(t_type);

public:
	union
	{
		struct {t_type x, y, z, w;};
		struct {t_type r, g, b, a;};
	};

	vector4(t_type inX, t_type inY, t_type inZ, t_type inW) : x(inX), y(inY), z(inZ), w(inW) {}
	vector4(const vector2<t_type> &vec, t_type inZ = static_cast<t_type>(0), t_type inW = static_cast<t_type>(0)) 
		: vector4(vec.x, vec.y, inZ, inW) {}
	vector4(const vector3<t_type> &vec, t_type inW = static_cast<t_type>(0)) : vector4(vec.x, vec.y, vec.z, inW) {}
	vector4(t_type value) : vector4(value, value, value, value) {}
	vector4() :	vector4(static_cast<t_type>(0)) {}

	explicit operator vector2<t_type>() const 
	{ 
		return vector2<t_type>(x, y); 
	}

	explicit operator vector3<t_type>() const
	{
		return vector3<t_type>(x, y, z);
	}

	void operator = (const vector4<t_type> &vec)
	{
		x = vec.x;
		y = vec.y;
		z = vec.z;
		w = vec.w;
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
	auto operator + (const vector4<t_rightType> &vec) const
	{
		g_assertIsArithmetic(t_rightType);
		using resultType = decltype(x + vec.x);

		return vector4<resultType>(x + vec.x, y + vec.y, z + vec.z, w + vec.w);
	}

	template<typename t_rightType>
	auto operator - (const vector4<t_rightType> &vec) const
	{
		g_assertIsArithmetic(t_rightType);
		using resultType = decltype(x - vec.x);

		return vector4<resultType>(x - vec.x, y - vec.y, z - vec.z, w - vec.w);
	}

	vector4<t_type> operator - () const
	{
		return vector4<t_type>(-x, -y, -z, -w);
	}

	template<typename t_scalarType>
	auto operator * (t_scalarType value) const
	{
		g_assertIsArithmetic(t_scalarType);
		using resultType = decltype(x * value);

		return vector4<resultType>(x * value, y * value, z * value, w * value);
	}

	template<typename t_scalarType>
	vector4<float> operator / (t_scalarType value) const
	{
		g_assertIsArithmetic(t_scalarType);
		const float divisor = static_cast<float>(value);

		return vector4<float>(x / divisor, y / divisor, z / divisor, w / divisor);
	}

	bool operator == (const vector4<t_type> &vec) const
	{
		return (x == vec.x) && (y == vec.y) && (z == vec.z) && (w == vec.w);
	}
};

template<typename t_vectorType, typename t_scalarType>
static auto operator * (t_scalarType scalar, const vector4<t_vectorType> &vec)
{
	g_assertIsArithmetic(t_scalarType);
	return vec * scalar;
}

template<typename t_type>
static bool operator != (const vector4<t_type> vecA, const vector4<t_type> &vecB)
{
	return !(vecA == vecB);
}

template<typename t_type>
static std::ostream & operator << (std::ostream &os, const vector4<t_type> &vec)
{
	os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
	return os;
}

template <typename t_type>
static std::istream & operator >> (std::istream &is, vector4<t_type> &vec)
{
	is >> vec.x >> vec.y >> vec.z >> vec.w;
	return is;
}

template<typename t_type>
static float sqrLength(const vector4<t_type> &vec)
{
	return static_cast<float>((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z) + (vec.w * vec.w));
}

template<typename t_type>
static float length(const vector4<t_type> &vec)
{
	return sqrtf(sqrLength(vec));
}

template<typename t_type>
static t_type dot(const vector4<t_type> &vecA, const vector4<t_type> &vecB)
{
	return (vecA.x * vecB.x) + (vecA.y * vecB.y) + (vecA.z * vecB.z) + (vecA.w * vecB.w);
}

template<typename t_type>
static vector4<float> normalize(const vector4<t_type> &vec)
{
	float len = length(vec);
	len = (len == 0) ? 1 : len;
	
	return static_cast<vector4<float>>(vec) / len;
}

using vec4	= vector4<float>;
using vec4i = vector4<i32>;

using color4 = vector4<float>;
using color4i = vector4<u8>;

