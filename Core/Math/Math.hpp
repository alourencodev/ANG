#pragma once

#include "Core/Attributes.hpp"
#include "Core/Types.hpp"
#include "Core/Math/Values.hpp"


namespace age::math
{

template<typename t_type>
_force_inline constexpr auto lerp(const t_type &valA, const t_type &valB, float t)
{
	auto delta = valB - valA;
	return valA + (delta * t);
}


/**
@brief Returns the next power of 2 that is greater than n. If n is a power of 2, it will return itself.
**/
constexpr u64 nextPow2(u64 n)
{
	if (n < 2)
		return 2;

	n--;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n |= n >> 32;
	n++;

	return n;
}


template<typename t_type>
_force_inline constexpr t_type max(t_type a, t_type b)
{
	return b > a ? b : a;
}

template<typename t_type>
_force_inline constexpr t_type min(t_type a, t_type b)
{
	return a > b ? b : a; 
}

template<typename t_type>
_force_inline constexpr t_type clamp(t_type value, t_type minValue, t_type maxValue)
{
  return min(maxValue, max(value, minValue));
}


_force_inline constexpr i32 abs(i32 n)
{
	constexpr u8 k_byteSize = 8;
	i32 mask = n >> (sizeof(i32) * k_byteSize - 1);
	return (mask + n) ^ mask;
}

_force_inline f32 fabs(f32 n)
{
	u32 i = reinterpret_cast<const u32&>(n);
	i &= 0x7FFFFFFF;
	
	return reinterpret_cast<const f32&>(i);
}


_force_inline bool approx(float a, float b)
{
	return fabs(a - b) < k_epsilon;
}


_force_inline constexpr byte indexToByte(u8 index) 
{ 
	return 1 << index; 
}

}	// namespace age::math
