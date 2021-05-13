#pragma once

#include <string>

#include "Core/Attributes.hpp"


namespace logger
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
static const LogSetting k_warnLogSetting = {k_yellowColor, 'W', false};
static const LogSetting k_errorLogSetting = {k_redColor, 'E', false};

void rawLog(const LogSetting &setting, const char *file, long line, const std::string &tag, const char *format, ...);
void enable(const std::string &tag);
void disable(const std::string &tag);

}

#ifdef _DEBUG
#	define g_log(tag, ...) logger::rawLog(logger::k_debugLogSetting, __FILE__, __LINE__, tag, __VA_ARGS__)
#else
#	define g_log(tag, ...) ;
#endif

#ifdef _RELEASE_SYMB
#	define g_warning(tag, ...) logger::rawLog(logger::k_warnLogSetting, __FILE__, __LINE__, tag, __VA_ARGS__)
#	define g_error(tag, ...) { logger::rawLog(logger::k_errorLogSetting, __FILE__, __LINE__, tag, __VA_ARGS__); _breakpoint(); std::exit(EXIT_FAILURE); }
#else
#	define g_warning(tag, ...) ;
#	define g_error(tag, ...) ;
#endif

