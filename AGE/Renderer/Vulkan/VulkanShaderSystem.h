#pragma once

#include <vulkan/vulkan.h>

#include <Core/DArray.hpp>
#include <Core/Handle.hpp>

DECLARE_HANDLE(ShaderHandle);

namespace age::vk
{

enum class e_ShaderStage : u8
{
	Vertex,
	Fragment,

	Count
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
