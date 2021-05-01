#pragma once

#include <cmath>
#include <iostream>

#include "Log/Assert.h"
#include "Types.hpp"

template<typename t_type>
union vector2
{
	assertIsArithmetic(t_type);

public:
	union
	{
		struct {t_type x, y;};
		struct {t_type w, h;};
	};

	vector2(t_type inX, t_type inY) : x(inX), y(inY) {}
	vector2(t_type value) : vector2(value, value) {}
	vector2() :	vector2(static_cast<t_type>(0)) {}

	void operator = (const vector2<t_type> &vec)
	{
		x = vec.x;
		y = vec.y;
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
	auto operator + (const vector2<t_rightType> &vec) const
	{
		assertIsArithmetic(t_rightType);
		using resultType = decltype(x + vec.x);

		return vector2<resultType>(x + vec.x, y + vec.y);
	}

	template<typename t_rightType>
	auto operator - (const vector2<t_rightType> &vec) const
	{
		assertIsArithmetic(t_rightType);
		using resultType = decltype(x - vec.x);

		return vector2<resultType>(x - vec.x, y - vec.y);
	}

	vector2<t_type> operator - () const
	{
		return vector2<t_type>(-x, -y);
	}

	template<typename t_scalarType>
	auto operator * (t_scalarType value) const
	{
		assertIsArithmetic(t_scalarType);

		using resultType = decltype(x * value);

		return vector2<resultType>(x * value, y * value);
	}

	template<typename t_scalarType>
	vector2<float> operator / (t_scalarType value) const
	{
		assertIsArithmetic(t_scalarType);
		const float divisor = static_cast<float>(value);

		return vector2<float>(x / divisor, y / divisor);
	}

	bool operator == (const vector2<t_type> &vec) const
	{
		return (x == vec.x) && (y == vec.y);
	}
};

template<typename t_vectorType, typename t_scalarType>
static auto operator * (t_scalarType scalar, const vector2<t_vectorType> &vec)
{
	assertIsArithmetic(t_scalarType);
	return vec * scalar;
}

template<typename t_type>
static bool operator != (const vector2<t_type> vecA, const vector2<t_type> &vecB)
{
	return !(vecA == vecB);
}

template<typename t_type>
static std::ostream & operator << (std::ostream &os, const vector2<t_type> &vec)
{
	os << "(" << vec.x << ", " << vec.y << ")";
	return os;
}

template <typename t_type>
static std::istream & operator >> (std::istream &is, vector2<t_type> &vec)
{
	is >> vec.x >> vec.y;
	return is;
}

template<typename t_type>
static float sqrLength(const vector2<t_type> &vec)
{
	return static_cast<float>((vec.x * vec.x) + (vec.y * vec.y));
}

template<typename t_type>
static float length(const vector2<t_type> &vec)
{
	return sqrtf(sqrLength(vec));
}

template<typename t_type>
static t_type dot(const vector2<t_type> &vecA, const vector2<t_type> &vecB)
{
	return (vecA.x * vecB.x) + (vecA.y * vecB.y);
}

template<typename t_type>
static vector2<float> normalize(const vector2<t_type> &vec)
{
	float len = length(vec);
	len = (len == 0) ? 1 : len;
	
	return static_cast<vector2<float>>(vec) / len;
}

using vec2	= vector2<float>;
using vec2i = vector2<i32>;

using size	= vector2<float>;
using sizei = vector2<u32>;

