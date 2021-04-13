#pragma once

#include "Types.hpp"


template<typename t_type>
constexpr auto lerp(const t_type &valA, const t_type &valB, float t)
{
	auto delta = valB - valA;
	return valA + (delta * t);
}


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
