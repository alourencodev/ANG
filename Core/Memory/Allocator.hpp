#pragma once

#include "Meta.hpp"

template<typename t_type>
class DefaultHeapAllocator
{
public:
	static t_type *alloc(size_t count) { return reinterpret_cast<t_type *>(std::malloc(sizeof(t_type) * count)); }
	static void dealloc(t_type *ptr) { std::free(ptr); }
	static bool realloc(t_type **outPtr, size_t count) 
	{ 
		if (count == 0) {
			// This is explicitly done since we don't know what the current realloc implementation does.
			dealloc(*outPtr);
			*outPtr = nullptr;
		} 
		else {
			t_type *newPtr = reinterpret_cast<t_type *>(std::realloc(*outPtr, sizeof(t_type) * count)); 
			if (newPtr == nullptr) {
				g_warning(k_tag, "Failed to reallocate memory with size %d", count);
				return false;
			}

			*outPtr = newPtr;
		}

		return true;
	}
private:
	constexpr static char k_tag[] = "DefaultHeapAllocator";
};
