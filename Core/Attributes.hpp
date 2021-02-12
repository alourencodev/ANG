#pragma once

#ifdef _MSC_VER

#include <intrin.h>

#define _force_inline __forceinline
#define _breakpoint() __debugbreak()
#else
#warning ("Attributes are only available for MSVC.")
#define _force_inline
#define _breakpoint()
#endif

#define _nodiscard [[nodiscard]]
