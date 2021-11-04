#include <AGE/Renderer/Renderer.h>

#include <AGE/Renderer/Vulkan/VulkanPipelineSystem.h>
#include <AGE/Renderer/Vulkan/VulkanShaderSystem.h>

namespace age
{

Renderer Renderer::s_inst = Renderer();

void Renderer::init(GLFWwindow *window)
{
	vk::VulkanSystem::s_inst.init(window);

	vk::ShaderSystem &shaderSystem = vk::ShaderSystem::s_inst;

	// TODO: Remove shader test code
	ShaderHandle vertexShader = shaderSystem.createShader(vk::e_ShaderStage::Vertex, "Shaders/dummy.vert.spv");
	ShaderHandle fragmentShader = shaderSystem.createShader(vk::e_ShaderStage::Fragment, "Shaders/dummy.frag.spv");

	vk::PipelineSystem::CreateInfo info = {};
	info.shaders.add(shaderSystem.get(vertexShader));
	info.shaders.add(shaderSystem.get(fragmentShader));
	PipelineHandle pipelineHandle = vk::PipelineSystem::s_inst.createPipeline(info);

	_testCommandBuffers = vk::VulkanSystem::s_inst.allocDrawCommandBuffer(pipelineHandle);
}

void Renderer::update()
{
	// TODO
	vk::VulkanSystem::s_inst.draw(_testCommandBuffers);
}

void Renderer::cleanup()
{
	vk::VulkanSystem &vkSystem = vk::VulkanSystem::s_inst;

	vkSystem.waitForAllFrames();

	vkSystem.freeDrawCommandBuffers(_testCommandBuffers);

	vk::PipelineSystem::s_inst.cleanup();
	vk::ShaderSystem::s_inst.cleanup();
	vkSystem.cleanup();
}

}