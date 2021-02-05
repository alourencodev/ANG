#pragma once

#ifdef _MSC_VER

#include <intrin.h>

#define force_inline __forceinline
#define breakpoint() __debugbreak()
#else
#warning ("Attributes are only available for MSVC.")
#define force_inline
#define breakpoint()
#endif
