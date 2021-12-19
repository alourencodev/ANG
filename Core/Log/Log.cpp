#include "Log.h"

#include <cstdarg>
#include <cstdio>
#include <iostream>
#include <set>

#include "Core/Types.hpp"

namespace age::logger
{

static std::set<std::string> s_loggerRegister;

constexpr u16 k_maxLogSize = 1024;
constexpr char k_resetColor[] = "\033[0m";

void rawLog(const LogSetting &setting, const char *file, long line, const std::string &tag, const char *format, ...)
{
    if (setting.needsRegister && (s_loggerRegister.find(tag) == s_loggerRegister.end()))
        return;

    char buffer[k_maxLogSize];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    std::cout << setting.color << file << ":" << line << " " << setting.label << " [" << tag.c_str() << "] - " 
              << buffer << k_resetColor << std::endl;
}


void enable(const std::string &tag)
{
    if (s_loggerRegister.find(tag) != s_loggerRegister.end())
    {
        age_warning("Logger", "Tag %s is already enabled.", tag.c_str());
        return;
    }

    s_loggerRegister.insert(tag);
}

void disable(const std::string &tag)
{
    if (s_loggerRegister.find(tag) == s_loggerRegister.end())
    {
        age_warning("Logger", "Tag %s is already disabled.", tag.c_str());
		return;
    }

    s_loggerRegister.erase(tag);
}

}
