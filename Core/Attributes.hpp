#pragma once

#ifdef _MSC_VER
#define force_inline __forceinline
#else
#warning ("Attributes are only available for MSVC.")
#define force_inline
#endif
