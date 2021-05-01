#pragma once

#include "Log.h"
#include "Meta.hpp"


namespace logger
{

static const LogSetting k_assertLogSetting = {"A", k_yellowColor, false};

};


#if _DEBUG || _RELEASE_SYMB
#	define logAssert(exp) assertMsg(exp, "(%s) is not true.", #exp);
#	define logAssertMsg(exp, ...) if (!(exp)) { logger::rawLog(logger::k_assertLogSetting, __FILE__, __LINE__, "Assert", __VA_ARGS__); _breakpoint(); }
#	define logAssertFatal(exp, ...) if (!(exp)) { logError("Assert Fatal", __VA_ARGS__)}
#else
#	define logAssert(exp) ;
#	define logAssertMsg(exp, ...) ;
#	define logAssertFatal(exp, ...) ;
#endif


#define assertIsArithmetic(type) static_assert(meta::isArithmetic<type>::value, "Given type(" #type ") must be arithmetic!")
#define assertIsIntegral(type) static_assert(meta::isIntegral<type>::value, "Given type(" #type ") must be integral!")
