
#include <filesystem>

#include <Core/SArray.hpp>
#include <Core/StringMap.hpp>


constexpr char k_tag[] = "ShaderCompiler";


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

static const age::StringMap<EShaderStage> k_extensionMap = {
	{k_vertexExtension, EShaderStage::Vertex},
	{k_fragmentExtension, EShaderStage::Fragment}
};


// Output Extensions
constexpr char k_spirvExtension[] = "spv";
constexpr char k_reflexionExtension[] = "ageshader";


// Options
constexpr char k_includeTag[] = "-I";


int main(int argc, char *argv[])
{
	age_assertFatal(argc > 1, "No parameters were passed to the shader compiler.");

	const char *shaderSource = argv[1];
	EShaderStage shaderStage = EShaderStage::Invalid;

	{	// Parse input file
		auto sourcePath = std::filesystem::path(shaderSource);
		auto extensionStr = sourcePath.extension().string();
		const char *extension = extensionStr.c_str();

		if (k_extensionMap.contains(extension))
			shaderStage = k_extensionMap[extension];
		else
			age_error(k_tag, "Unable to compile shader $s due to invalid extension(%s)", shaderSource, extension);
	}

	{	// Parse Compilation Options
	
	}

	return 0;
}
