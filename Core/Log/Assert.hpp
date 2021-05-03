#pragma once

#include "Log.h"
#include "Meta.hpp"

namespace logger
{

static const LogSetting k_assertLogSetting = {"A", k_yellowColor, false};

};

#if _DEBUG || _RELEASE_SYMB
#	define g_assert(exp, ...) if (!(exp)) { age::logger::rawLog(age::logger::k_assertLogSetting, __FILE__, __LINE__, "Assert", __VA_ARGS__); _breakpoint(); }
#	define g_assertFatal(exp, ...) if (!(exp)) { g_error("Assert Fatal", __VA_ARGS__); }
#else
#	define g_assert(exp, ...) ;
#	define g_assertFatal(exp, ...) ;
#endif

#define g_assertIsArithmetic(type) static_assert(meta::isArithmetic<type>::value, "Given type(" #type ") must be arithmetic!")
#define g_assertIsIntegral(type) static_assert(meta::isIntegral<type>::value, "Given type(" #type ") must be integral!")
