#include "Compiler.h"

#include <Core/Log/Log.h>
#include <Core/StringUtils.hpp>

#include "Includer.h"



namespace age
{

constexpr char k_tag[] = "Compiler";

Compiler::Compiler()
{
	_compiler = shaderc_compiler_initialize();
	_options = shaderc_compile_options_initialize();

#ifdef AGE_DEBUG
	const char k_buildSchemeMacro[] = "AGE_DEBUG";
	shaderc_compile_options_set_generate_debug_info(_options);
#else
	const char k_buildSchemeMacro[] = "AGE_RELEASE";
#endif

	shaderc_compile_options_add_macro_definition(_options, k_buildSchemeMacro, strSize(k_buildSchemeMacro), NULL, 0);
	shaderc_compile_options_set_optimization_level(_options, shaderc_optimization_level_performance);
}



Compiler::~Compiler()
{
	shaderc_compile_options_release(_options);
	shaderc_compiler_release(_compiler);
}



void Compiler::bindIncluder(Includer &includer)
{
	shaderc_compile_options_set_include_callbacks(_options, resolveInclude, releaseIncludeResult, &includer);
}



Compiler::Result Compiler::compile(const DArray<byte>& sourceCode, const char *fileName, shaderc_shader_kind stage) const
{
	auto result = shaderc_compile_into_spv(_compiler, sourceCode.data(), sourceCode.count(), stage, fileName, "main", _options);
	
	if (shaderc_result_get_num_errors(result) > 0)
		age_error(k_tag, shaderc_result_get_error_message(result));

	auto compilationStatus = shaderc_result_get_compilation_status(result);
	switch(compilationStatus) {
		case shaderc_compilation_status_success:
			// Do Nothing
			break;
		case shaderc_compilation_status_invalid_stage:
			age_error(k_tag, "Compilation failed due to 'Invalid Stage'");
			break;
		case shaderc_compilation_status_compilation_error:
			age_error(k_tag, "Compilation failed due to 'Compilation Error'");
			break;
		case shaderc_compilation_status_internal_error:
			age_error(k_tag, "Compilation failed due to 'Internal Error'");
			break;

		case shaderc_compilation_status_null_result_object:
			age_error(k_tag, "Compilation failed due to 'Null Result Object'");
			break;

		case shaderc_compilation_status_invalid_assembly:
			age_error(k_tag, "Compilation failed due to 'Invalid Assembly'");
			break;

		case shaderc_compilation_status_validation_error:
			age_error(k_tag, "Compilation failed due to 'Validation Error'");
			break;

		case shaderc_compilation_status_transformation_error:
			age_error(k_tag, "Compilation failed due to 'Transformation Error'");
			break;

		case shaderc_compilation_status_configuration_error:
			age_error(k_tag, "Compilation failed due to 'Configuration Error'");
			break;
		default:
			age_error(k_tag, "Trying to handle unknown compilation status.");
	}

	size_t binSize = shaderc_result_get_length(result);
	const char *bin = shaderc_result_get_bytes(result);

	Result compilerResult = {};
	compilerResult._bin = Range<const char>(bin, binSize);
	compilerResult._result = result;

	return compilerResult;
}

}