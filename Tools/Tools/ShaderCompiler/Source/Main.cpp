
#include <filesystem>

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



enum class EShaderStage : u8
{
	Invalid,
	Vertex,
	Fragment,

	Count
};

struct Shader
{
	fs::path sourcePath;
	fs::path outputPath;
	EShaderStage stage;
};

static const age::StringMap<EShaderStage> k_extensionMap = {
	{k_vertexExtension, EShaderStage::Vertex},
	{k_fragmentExtension, EShaderStage::Fragment}
};

static const age::SArray<const char*, 2> k_validExtensions = {
	k_vertexExtension,
	k_fragmentExtension
};



// Output Extensions
constexpr char k_spirvExtension[] = "spv";
constexpr char k_reflexionExtension[] = "ageshader";



void listShaders(const char *dir, const fs::path &outputDir, DArray<Shader>& o_shaders)
{
	DArray<fs::path> directories = {};

	for (const auto& entry : fs::directory_iterator(dir)) {
		if (entry.is_directory()) { 
			directories.add(entry.path());
			continue;
		}

		auto extensionString = entry.path().extension().string();
		const char* extension = extensionString.c_str();

		Shader shader = {};
		shader.sourcePath = entry.path();
		shader.outputPath = outputDir;
		shader.stage = k_extensionMap[extension];

		age_log(k_verboseTag, "Shader: %s", shader.sourcePath.string().c_str());
		o_shaders.add(shader);
	}

	for (const auto& subdir : directories) {
		const u32 rootDirectorySize = ConstStringView(dir).calcSize();

		fs::path outputSubdir = outputDir;
		outputSubdir.concat(&dir[rootDirectorySize]);
		listShaders(subdir.string().c_str(), outputSubdir, o_shaders);
	}
}



int main(int argc, char *argv[])
{
	// Expected argument layout:
	// SahaderCompiler <Source_Dir> <Output_Dir> <Include_Dir_1> <Include_Dir_2>

#ifdef AGE_DEBUG
	age::logger::enable(k_verboseTag);
#endif

	age::logger::enable(k_tag);
	age_assertFatal(argc > 1, "No parameters were passed to the shader compiler.");
	age_log(k_tag, "Compiling Shaders...");

	Timer compilationTimer;
	compilationTimer.start();

	const u8 includeDirectoryCount = argc - 2;
	const char *sourceDir = argv[1];
	const char *outputDir = argv[2];

	age_log(k_verboseTag, "SourceDir: %s", sourceDir);
	age_log(k_verboseTag, "OutputDir: %s", outputDir);

	DArray<const char*> includeDirectories = {};
	includeDirectories.reserve(includeDirectoryCount);

	// Store include directories
	for (int i = 3; i < argc; i++) {
		age_log(k_verboseTag, "IncludeDir: %s", argv[i]);
		includeDirectories.add(argv[i]);
	}
	
	DArray<Shader> shaders = {};
	listShaders(sourceDir, fs::path(outputDir), shaders);

	age_log(k_tag, "Shader compilation complete. Duration %.2f", (compilationTimer.millis() / 1000.0f));

	return 0;
}
