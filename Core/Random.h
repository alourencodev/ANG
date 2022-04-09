#pragma once

#include <Core/Attributes.hpp>
#include <Core/Types.hpp>

#include <random>


namespace age
{

template<typename t_type = i32>
_force_inline t_type randomInt(t_type min, t_type max)
{
	std::random_device device;
	std::uniform_int_distribution<t_type> distribution(min, max);

	return distribution(device);
}

}
