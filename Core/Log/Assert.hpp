#pragma once

#include "Core/BuildScheme.hpp"
#include "Core/Log/Log.h"
#include "Core/Meta.hpp"

namespace age::logger
{

static const LogSetting k_assertLogSetting = {k_yellowColor, 'A', false};

};

#ifdef AGE_ASSERT_ENABLED
#	define age_assert(exp, ...) if (!(exp)) { age::logger::rawLog(age::logger::k_assertLogSetting, __FILE__, __LINE__, "Assert", __VA_ARGS__); _breakpoint(); }
#	define age_assertFatal(exp, ...) if (!(exp)) { age_error("Assert Fatal", __VA_ARGS__); }
#else
#	define age_assert(exp, ...) ;
#	define age_assertFatal(exp, ...) ;
#endif

#define age_assertIsArithmetic(type) static_assert(meta::isArithmetic<type>::value, "Given type(" #type ") must be arithmetic!")
#define age_assertIsIntegral(type) static_assert(meta::isIntegral<type>::value, "Given type(" #type ") must be integral!")
