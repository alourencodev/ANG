#pragma once

#include <cstdint> // Using the standard as proxy to keep multiplaform
#include <type_traits>

// Integers
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

// Unsigned Integers
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

// Other
using byte = u8;

#define ASSERT_IS_ARITHMETIC(type) static_assert(std::is_arithmetic<type>::value, "Given type must be arithmetic!")
#define ASSERT_IS_INTEGRAL(type) static_assert(std::is_integral<type>::value, "Given type must be integral!")

