#pragma once

#include <string>

#include "Core/Attributes.hpp"
#include "Core/BuildScheme.hpp"


namespace age::logger
{

struct LogSetting
{
	std::string color;
	char label;
	bool needsRegister;
};

constexpr char k_defaultColor[] = "";
constexpr char k_yellowColor[] = "\033[33m";
constexpr char k_redColor[] = "\033[31m";

static const LogSetting k_debugLogSetting = {k_defaultColor, 'D', true};
static const LogSetting k_forcedDebugLogSetting = {k_defaultColor, 'D', false};
static const LogSetting k_warnLogSetting = {k_yellowColor, 'W', false};
static const LogSetting k_errorLogSetting = {k_redColor, 'E', false};

void rawLog(const LogSetting &setting, const char *file, long line, const std::string &tag, const char *format, ...);
void enable(const std::string &tag);
void disable(const std::string &tag);

}

#ifdef AGE_LOG_ENABLED
#	define age_log(tag, ...) age::logger::rawLog(age::logger::k_debugLogSetting, __FILE__, __LINE__, tag, __VA_ARGS__)
#	define age_forceLog(tag, ...) age::logger::rawLog(age::logger::k_forcedDebugLogSetting, __FILE__, __LINE__, tag, __VA_ARGS__)
#	define age_warning(tag, ...) age::logger::rawLog(age::logger::k_warnLogSetting, __FILE__, __LINE__, tag, __VA_ARGS__)
#	define age_error(tag, ...) { age::logger::rawLog(age::logger::k_errorLogSetting, __FILE__, __LINE__, tag, __VA_ARGS__); _breakpoint(); std::exit(EXIT_FAILURE); }
#else
#	define age_log(tag, ...) ;
#	define age_forceLog(tag, ...) ;
#	define age_warning(tag, ...) ;
#	define age_error(tag, ...) ;
#endif

