#pragma once

// Compiler Attributes
#if defined(_MSC_VER)
#	include <intrin.h>
#	define _force_inline __forceinline
#elif defined(__GNUC__)
#	define _force_inline __attribute__((always_inline)) inline
#else
#	warning ("Attributes are only available for MSVC.")
#	define _force_inline
#endif


// Dev Environment Attributes
#ifdef _MSC_VER
#	define _breakpoint() __debugbreak()
#else
#	define _breakpoint()
#endif


#define _nodiscard [[nodiscard]]
