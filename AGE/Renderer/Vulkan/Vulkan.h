#pragma once

#include <vulkan/vulkan.h>

#include <Core/DArray.hpp>
#include <Core/Handle.hpp>
#include <Core/StackArray.hpp>

struct GLFWwindow;

namespace age::vk
{

DECLARE_HANDLE(ShaderHandle);
DECLARE_HANDLE(PipelineHandle);

enum class e_ShaderStage : u8
{
	Vertex,
	Fragment,

	Count
};


using ShaderArray = StackArray<ShaderHandle, static_cast<u32>(e_ShaderStage::Count)>;

struct PipelineCreateInfo
{
	ShaderArray shaders;
};


void init(GLFWwindow *window);
void cleanup();
void recreateRenderEnvironment();
void draw(const DArray<VkCommandBuffer> &commandBuffers);

ShaderHandle createShader(e_ShaderStage shaderStage, const char *path);
PipelineHandle createPipeline(const PipelineCreateInfo &info);

} // namespace age::vk