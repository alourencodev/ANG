#pragma once

#include <shaderc/shaderc.h>

#include <Core/DArray.hpp>
#include <Core/String.hpp>
#include <Core/HashMap.hpp>



namespace age
{

shaderc_include_result *resolveInclude(void *userData, const char *requestedSource, int type, const char *requestingSource, size_t includeDepth);
void releaseIncludeResult(void *userData, shaderc_include_result *includeResult);



class Includer
{
public:
	Includer() = default;

	_force_inline void reserveIncludeDir(size_t count) { _includeDirs.reserve(count); }
	_force_inline void addIncludeDir(const char *includeDir) { _includeDirs.add(includeDir); }

	const String *getSource(const char *requestedSource);

private:

	DArray<const char *> _includeDirs = {};
	HashMap<String, String> _sourceMap = {};
};

}
