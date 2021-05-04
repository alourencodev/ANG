#pragma once

#include <Core/Platform.hpp>

#ifdef _WINDOWS
#	ifdef _AGE_BUILD
#		define AGE_API __declspec(dllexport)
#	else
#		define AGE_API __declspec(dllimport)
#	endif
#else
#	error AGE Doesn't Support the current platform
#endif
