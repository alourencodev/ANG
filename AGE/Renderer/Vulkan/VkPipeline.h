#pragma once

#include <vulkan/vulkan.h>

#include <AGE/Renderer/Vulkan/VkShader.h>

#include <Core/Handle.hpp>


namespace age::vk
{

struct RenderTarget;
struct Swapchain;

DECLARE_HANDLE(PipelineHandle);


struct Pipeline
{
	VkAllocationCallbacks *allocator = nullptr;
	VkPipelineLayout layout = {};
	VkViewport viewport = {};
	VkRect2D scissor = {};
	ShaderHandleArray shaderHandles = {};
	VkPipeline pipeline = VK_NULL_HANDLE;
};



struct PipelineCreateInfo
{
	VkAllocationCallbacks *allocator = nullptr;
	ShaderHandleArray shaderHandles;
};



Pipeline createPipeline(const Context &context, const Swapchain &swapchain, const RenderTarget &renderTarger, const DArray<Shader> &shaders, const PipelineCreateInfo &info);
void destroyPipeline(const Context &context, Pipeline &pipeline);

}
