#pragma once

#include <cstdint> // Using the standard as proxy to keep multiplaform
#include <type_traits>

// Integers
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

// Unsigned Integers
using ui8 = uint8_t;
using ui16 = uint16_t;
using ui32 = uint32_t;
using ui64 = uint64_t;

// Other
using byte = ui8;

#define ASSERT_IS_ARITHMETIC(type) static_assert(std::is_arithmetic<type>::value, "Given type must be arithmetic!")

