#pragma once

#include <string>

#include "Attributes.hpp"


namespace logger
{

struct LogSetting
{
	std::string label;
	std::string color;
	bool needsRegister;
};

constexpr char k_defaultColor[] = "";
constexpr char k_yellowColor[] = "\033[33m";
constexpr char k_redColor[] = "\033[31m";

static const LogSetting k_debugLogSetting = {"D", k_defaultColor, true};
static const LogSetting k_warnLogSetting = {"W", k_yellowColor, false};
static const LogSetting k_errorLogSetting = {"E", k_redColor, false};

void rawLog(const LogSetting &setting, const char *file, long line, const std::string &tag, const char *format, ...);
void enable(const std::string &tag);
void disable(const std::string &tag);

}

#ifdef _DEBUG
#	define logDebug(tag, ...) logger::rawLog(logger::k_debugLogSetting, __FILE__, __LINE__, tag, __VA_ARGS__)
#else
#	define logDebug(tag, ...) ;
#endif

#if _DEBUG || _RELEASE_SYMB
#	define logWarning(tag, ...) logger::rawLog(logger::k_warnLogSetting, __FILE__, __LINE__, tag, __VA_ARGS__)
#	define logError(tag, ...) { logger::rawLog(logger::k_errorLogSetting, __FILE__, __LINE__, tag, __VA_ARGS__); _breakpoint(); std::exit(EXIT_FAILURE); }
#else
#	define logWarning(tag, ...) ;
#	define logError(tag, ...) ;
#endif

