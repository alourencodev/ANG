#pragma once

#include <vulkan/vulkan.h>

#include <Core/HashMap.hpp>
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

	const Shader &get(ShaderHandle handle) const { return _shadersMap[static_cast<u32>(handle)]; }

private:
	HashMap<u32, Shader> _shadersMap;
};

}
