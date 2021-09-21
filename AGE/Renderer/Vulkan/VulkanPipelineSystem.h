#pragma once

#include <Core/DArray.hpp>
#include <Core/StackArray.hpp>
#include <Core/Handle.hpp>

#include <AGE/Renderer/Vulkan/VulkanShaderSystem.h>

#include <vulkan/vulkan.h>

DECLARE_HANDLE(PipelineHandle);

namespace age::vk
{

struct Pipeline
{
	VkPipelineLayout layout = {};
	VkViewport viewport = {};
	VkRect2D scissor = {};
	VkPipeline pipeline = VK_NULL_HANDLE;
	VkRenderPass renderPass = VK_NULL_HANDLE;
};

class PipelineSystem
{
public:
	struct CreateInfo
	{
		StackArray<Shader, static_cast<u32>(e_ShaderStage::Count)> shaders; 
	};

	static PipelineSystem s_inst;

	PipelineHandle createPipeline(const CreateInfo &info);
	void cleanup();

	Pipeline get(PipelineHandle handle) { return _pipelines[static_cast<u32>(handle)]; }

private:
	DArray<Pipeline> _pipelines;

	VkPipelineShaderStageCreateInfo createShaderStage(const Shader &shader);
};

}	// namespace age::vk
