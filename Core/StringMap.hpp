#pragma once

#include <Core/HashMap.hpp>
#include <Core/StringView.hpp>

namespace age
{

namespace
{

// Values recommended at http://isthe.com/chongo/tech/comp/fnv/
constexpr u32 k_prime = 16777619; 
constexpr u32 k_seed  = 2166136261;

class StringHashMapBehavior
{
public:
	bool isEqual(const char *a, const char *b) const { return ConstStringView(a) == ConstStringView(b); }

	size_t hash(const char *value) const 
	{ 
		// Apply Fowler-Noll-Vo

		age_assertFatal(value != nullptr, "Cannot hash a nullptr string");
		size_t hash = k_seed;
		for (u32 i = 0; value[i] != '\0'; i++) 
			hash = (value[i] ^  hash) * k_prime;

		return hash;
	} 
};

}

template<typename t_valueType, typename t_allocator = DefaultHashMapAllocator<const char *, t_valueType>>
using StringMap = HashMap<const char *, t_valueType, StringHashMapBehavior, t_allocator>;

}
