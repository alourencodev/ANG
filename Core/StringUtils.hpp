#pragma once

#include <string>

#include "Attributes.hpp"

namespace age
{

_force_inline size_t strSize(const char *str)
{
	size_t len = 0;
	for (; str[len]; len++);

	return len;
}



_force_inline void copyStr(char *dst, const char *src)
{
	size_t size = strSize(src);
	memcpy(dst, src, size + 1);
}

}	// namespace age
