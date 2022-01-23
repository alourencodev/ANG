#pragma once

#include <shaderc/shaderc.h>

#include <Core/DArray.hpp>
#include <Core/Range.hpp>

namespace age
{

class Includer;

class Compiler
{
public:
	Compiler();
	~Compiler();
	
	void bindIncluder(Includer &includer);
	void compile(const char *sourceDir, const char *fileName, shaderc_shader_kind stage, const char *outputDir) const;

private:
	void reflect(const char *bin, size_t size, const char *fileName) const;

	shaderc_compiler_t _compiler = nullptr;
	shaderc_compile_options_t _options = nullptr;
};

}
