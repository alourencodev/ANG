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



_force_inline bool strEqual(const char* str1, const char* str2)
{
	int i = 0;
	for (; str1[i] && str2[i]; i++) {
		if (str1[i] != str2[i])
			return false;
	}
	
	return str1[i] == str2[i];
}

}	// namespace age
