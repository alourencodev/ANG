#pragma once

#include <Core/Attributes.hpp>
#include <Core/Log/Assert.hpp>

#include <functional>



namespace age::hash
{

template<typename t_keyType>
_force_inline bool isKeyEqual(const t_keyType &a, const t_keyType &b)
{ 
	return a == b; 
}


template<>
_force_inline bool isKeyEqual<cStr>(const cStr &a, const cStr &b)
{
	i32 i = 0;
	for (; a[i] != '\0'; i++) {
		if (a[i] != b[i])
			return false;
	}

	return a[i] == b[i];
}


template<typename t_keyType> 
_force_inline u64 hash(const t_keyType &value)
{ 
	return std::hash<t_keyType>{}(value); 
}


template<>
_force_inline u64 hash::hash<cStr>(const cStr &value)
{
	// Values recommended at http://isthe.com/chongo/tech/comp/fnv/
	constexpr u32 k_prime = 16777619; 
	constexpr u32 k_seed  = 2166136261;

	// Apply Fowler-Noll-Vo
	age_assertFatal(value != nullptr, "Cannot hash a nullptr string");
	u64 hash = k_seed;
	for (u32 i = 0; value[i] != '\0'; i++) 
		hash = (value[i] ^  hash) * k_prime;

	return hash;
};


} // namespace age::hash
