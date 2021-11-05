#pragma once

#include <Core/StackArray.hpp>
#include <Core/HashMap.hpp>
#include <Core/Handle.hpp>

#include <AGE/Renderer/Vulkan/VulkanShaderSystem.h>

#include <vulkan/vulkan.h>

DECLARE_HANDLE(PipelineHandle);

namespace age::vk
{

using ShaderArray = StackArray<ShaderHandle, static_cast<u32>(e_ShaderStage::Count)>;

struct Pipeline
{
	struct CreateInfo
	{
		ShaderArray shaders;
	};

	VkPipelineLayout layout = {};
	VkViewport viewport = {};
	VkRect2D scissor = {};
	VkPipeline pipeline = VK_NULL_HANDLE;
	CreateInfo createInfo;
};

class PipelineSystem
{
public:
	static PipelineSystem s_inst;

	PipelineHandle createPipeline(const Pipeline::CreateInfo &info);
	void cleanup();

	const Pipeline &get(PipelineHandle handle) const { return _pipelinesMap[handle]; }

private:
	HashMap<u32, Pipeline> _pipelinesMap;

	VkPipelineShaderStageCreateInfo createShaderStage(const Shader &shader);
};

}	// namespace age::vk
