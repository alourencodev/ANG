#pragma once

template<typename t_type>
auto lerp(const t_type &valA, const t_type &valB, float t)
{
	auto delta = valB - valA;
	return valA + (delta * t);
}