
#include <filesystem>
#include <string>

#include <shaderc/shaderc.h>

#include <Core/BuildScheme.hpp>
#include <Core/DArray.hpp>
#include <Core/SArray.hpp>
#include <Core/StringMap.hpp>
#include <Core/StringView.hpp>
#include <Core/Timer.hpp>


using namespace age;
namespace fs = std::filesystem;

constexpr char k_tag[] = "ShaderCompiler";
constexpr char k_verboseTag[] = "DShaderCompiler";


// Input Extensions
constexpr char k_vertexExtension[] = ".vert";
constexpr char k_fragmentExtension[] = ".frag";



struct Shader
{
	const char *relativePath = nullptr;
	const char* fileName = nullptr;
	shaderc_shader_kind stage;
};

static const age::StringMap<shaderc_shader_kind> k_extensionMap = {
	{k_vertexExtension, shaderc_vertex_shader},
	{k_fragmentExtension, shaderc_fragment_shader}
};

static const age::SArray<const char*, 2> k_validExtensions = {
	k_vertexExtension,
	k_fragmentExtension
};



// Output Extensions
constexpr char k_spirvExtension[] = "spv";
constexpr char k_reflexionExtension[] = "ageshader";



// Globals
static u32 g_sourceDirLength = 0;
static u32 g_outputDirLength = 0;



void listShaders(const ConstStringView &sourceDir, const ConstStringView &outDir, DArray<Shader>& o_shaders)
{
	for (const auto& entry : fs::recursive_directory_iterator(sourceDir.str())) {
		if (entry.is_directory())
			continue;

		std::string extensionString = entry.path().extension().string();
		const char* extension = extensionString.c_str();
		const shaderc_shader_kind *stagePtr = k_extensionMap.at(extension);

		// Ignore files with invalid extensions
		if (stagePtr == nullptr)
			continue;

		Shader shader = {};

		{	// FileName
			std::string fileName = entry.path().filename().string();
			

		}

		shader.sourcePath = entry.path();
		shader.outputPath = outputDir;
		shader.stage = *stagePtr;

		age_log(k_verboseTag, "Shader: %s", shader.sourcePath.str());
		o_shaders.add(shader);
	}
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
	ConstStringView sourceDir(argv[1]);
	g_sourceDirLength = sourceDir.calcSize();
	
	ConstStringView outputDir(argv[2]);
	g_outputDirLength = outputDir.calcSize();
	
	age_log(k_verboseTag, "SourceDir: %s", sourceDir);
	age_log(k_verboseTag, "OutputDir: %s", outputDir);


	const u8 includeDirectoryCount = argc - 2;
	DArray<const char*> includeDirectories = {};
	includeDirectories.reserve(includeDirectoryCount);

	// Store include directories
	for (int i = 3; i < argc; i++) {
		age_log(k_verboseTag, "IncludeDir: %s", argv[i]);
		includeDirectories.add(argv[i]);
	}
	
	DArray<Shader> shaders = {};
	listShaders(sourceDir, outputDir, shaders);

	age_log(k_tag, "Shader compilation complete. Duration %.2f", (compilationTimer.millis() / 1000.0f));

	return 0;
}
