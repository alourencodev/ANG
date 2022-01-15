#pragma once

#include <shaderc/shaderc.h>

#include <Core/DArray.hpp>
#include <Core/Range.hpp>

namespace age
{

class Compiler
{
public:
	class Result
	{
	public:
		Result() = default;
		~Result() { if (_result != nullptr) shaderc_result_release(_result); }

		_force_inline const Range<const char> bin() const { return _bin; }

		friend class Compiler;

	private:
		Range<const char> _bin = {};
		shaderc_compilation_result_t _result = nullptr;
	};

	Compiler();
	~Compiler();
	
	Result compile(const DArray<byte> &sourceCode, const char *fileName, shaderc_shader_kind stage) const;

private:
	shaderc_compiler_t _compiler = nullptr;
	shaderc_compile_options_t _options = nullptr;
};

}
