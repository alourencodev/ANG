#pragma once

#include <Core/Platform.hpp>

#ifdef AGE_WINDOWS
#	ifdef AGE_BUILD
#		define AGE_API __declspec(dllexport)
#	else
#		define AGE_API __declspec(dllimport)
#	endif
#else
#	error AGE Doesn't Support the current platform
#endif
