#include <AGE/Renderer/Renderer.h>

#include <AGE/Renderer/Vulkan/VulkanPipelineSystem.h>
#include <AGE/Renderer/Vulkan/VulkanShaderSystem.h>

namespace age
{

Renderer Renderer::s_inst = Renderer();

constexpr char k_tag[] = "Renderer";

ShaderHandle vertexShader;
ShaderHandle fragmentShader;



void Renderer::init(GLFWwindow *window)
{
	vk::VulkanSystem::s_inst.init(window);

	vk::ShaderSystem &shaderSystem = vk::ShaderSystem::s_inst;

	// TODO: Remove shader test code
	vertexShader = shaderSystem.createShader(vk::e_ShaderStage::Vertex, "Shaders/dummy.vert.spv");
	fragmentShader = shaderSystem.createShader(vk::e_ShaderStage::Fragment, "Shaders/dummy.frag.spv");

	vk::Pipeline::CreateInfo info = {};
	info.shaders.add(vertexShader);
	info.shaders.add(fragmentShader);
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

void Renderer::onWindowResize(GLFWwindow *window, int width, int height)
{
	age_log(k_tag, "Window resized to %d x %d.", width, height);

	vk::PipelineSystem::s_inst.cleanup();
	vk::VulkanSystem::s_inst.recreateSwapchain();

	// TODO: Properly recreate the pipelines
	vk::ShaderSystem &shaderSystem = vk::ShaderSystem::s_inst;

	vk::Pipeline::CreateInfo info = {};
	info.shaders.add(vertexShader);
	info.shaders.add(fragmentShader);
	PipelineHandle pipelineHandle = vk::PipelineSystem::s_inst.createPipeline(info);
	_testCommandBuffers = vk::VulkanSystem::s_inst.allocDrawCommandBuffer(pipelineHandle);

}

}