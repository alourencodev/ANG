
#include <filesystem>
#include <string>

#include <Core/BuildScheme.hpp>
#include <Core/DArray.hpp>
#include <Core/File.h>
#include <Core/SArray.hpp>
#include <Core/String.hpp>
#include <Core/StringBuilder.hpp>
#include <Core/StringMap.hpp>
#include <Core/StringUtils.hpp>
#include <Core/Timer.hpp>

#include "Compiler.h"
#include "Extensions.hpp"
#include "Includer.h"



using namespace age;
namespace fs = std::filesystem;

constexpr char k_tag[] = "ShaderCompiler";
constexpr char k_verboseTag[] = "DShaderCompiler";





struct Shader
{
	char relativePath[128] = {};
	char fileName[64] = {};
	shaderc_shader_kind stage;
};



// Output Extensions
constexpr char k_spirvExtension[] = ".spv";
constexpr char k_reflexionExtension[] = ".ageshader";



// Globals
static ConstStringView g_sourceDir;
static size_t g_sourceDirLength = 0;

static ConstStringView g_outputDir;
static size_t g_outputDirLength = 0;




DArray<Shader> listShaders()
{
	DArray<Shader> shaderList = {};
	shaderList.reserve(16);

	for (const auto& entry : fs::recursive_directory_iterator(g_sourceDir.str())) {
		if (entry.is_directory())
			continue;

		std::string extension = entry.path().extension().string();
		const shaderc_shader_kind *stagePtr = k_extensionMap.at(extension.c_str());

		// Ignore files with invalid extensions
		if (stagePtr == nullptr)
			continue;

		{	// Fill shader struct
			Shader shader = {};

			std::string shaderPath = entry.path().parent_path().string();
			if (shaderPath.size() > g_sourceDirLength)
				copyStr(shader.relativePath, shaderPath.c_str() + g_sourceDirLength);

			std::string fileName = entry.path().filename().string();
			copyStr(shader.fileName, fileName.c_str());

			shader.stage = *stagePtr;

			age_log(k_verboseTag, "Shader: %s", shader.relativePath);
			shaderList.add(shader);
		}
	}

	shaderList.shrinkToFit();
	return shaderList;
}




void resolveShader(const Compiler &compiler, const Shader& shader)
{
	constexpr u8 k_segmentCount = 3;	// root + relativePath + filename + extension
	StringBuilder builder;
	builder.reserve(k_segmentCount);

	String sourceDir;
	String outputDir;
	String reflexionDir;

	{	// Build Source Dir
		builder.append(g_sourceDir);
		builder.append(shader.relativePath);
		builder.append("\\");
		builder.append(shader.fileName);

		sourceDir = builder.build();
	}


	{	// Build Output Dir
		builder.clear();
		builder.append(g_outputDir);
		builder.append(shader.relativePath);
		builder.append("\\");
		builder.append(shader.fileName);
		builder.append(k_spirvExtension);

		outputDir = builder.build();

		fs::path outputDirPath(outputDir.str());
		if (!fs::exists(outputDirPath.parent_path()))
			fs::create_directory(outputDirPath.parent_path());
	}


	{	// Reflexion Output Dir
		builder.pop();
		builder.append(k_reflexionExtension);

		reflexionDir = builder.build();
	}
	
	compiler.compile(sourceDir, shader.fileName, shader.stage, outputDir);
}



void resolveShaderList(Includer &includer, const DArray<Shader>& shaders)
{
	Compiler compiler;
	compiler.bindIncluder(includer);

	for (const Shader &shader : shaders)
		resolveShader(compiler, shader);
}



int main(int argc, char *argv[])
{
	// Expected argument layout:
	// ShaderCompiler <Source_Dir> <Output_Dir> <Include_Dir_1> <Include_Dir_2>

#ifdef AGE_DEBUG
	age::logger::enable(k_verboseTag);
#endif

	age::logger::enable(k_tag);
	age_assertFatal(argc > 1, "No parameters were passed to the shader compiler.");
	age_log(k_tag, "Compiling Shaders...");

	Timer compilationTimer;
	compilationTimer.start();


	// Setup io dirs
	g_sourceDir = ConstStringView(argv[1]);
	g_sourceDirLength = g_sourceDir.calcSize();
	
	g_outputDir = ConstStringView(argv[2]);
	g_outputDirLength = g_outputDir.calcSize();
	
	age_log(k_verboseTag, "SourceDir: %s", g_sourceDir.str());
	age_log(k_verboseTag, "OutputDir: %s", g_outputDir.str());

	const u8 includeDirectoryCount = argc - 2;
	Includer includer;
	includer.reserveIncludeDir(includeDirectoryCount);

	// Store include directories
	for (int i = 3; i < argc; i++) {
		age_log(k_verboseTag, "IncludeDir: %s", argv[i]);
		includer.addIncludeDir(argv[i]);
	}
	
	DArray<Shader> shaders = listShaders();
	resolveShaderList(includer, shaders);

	age_log(k_tag, "Shader compilation complete. Duration %.2f", (compilationTimer.millis() / 1000.0f));

	return 0;
}
