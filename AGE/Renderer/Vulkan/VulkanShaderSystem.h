#pragma once

#include <Core/BuildScheme.hpp>
#include <Core/DArray.hpp>
#include <Core/Handle.hpp>

#include <vulkan/vulkan.h>

DECLARE_HANDLE(ShaderHandle);

template<typename,typename>
class age::DArray;

namespace age::vk
{

enum class e_ShaderStage : u8
{
	Vertex,
	Fragment
};

struct Shader
{
	VkShaderModule module;
	VkShaderStageFlagBits stage;
};

class ShaderSystem
{
public:
	static ShaderSystem s_inst;

	ShaderHandle createShader(e_ShaderStage shaderStage, const char *path);
	void cleanup();

	Shader get(ShaderHandle handle) { return _shaders[static_cast<u32>(handle)]; }

private:
	DArray<Shader> _shaders;
};

}
