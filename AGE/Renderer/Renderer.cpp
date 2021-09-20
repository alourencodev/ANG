#include <AGE/Renderer/Renderer.h>

#include <AGE/Renderer/Vulkan/VulkanPipelineSystem.h>
#include <AGE/Renderer/Vulkan/VulkanShaderSystem.h>
#include <AGE/Renderer/Vulkan/VulkanSystem.h>

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
	PipelineHandle pipeline = vk::PipelineSystem::s_inst.createPipeline(info);
}

void Renderer::update()
{
	// TODO
}

void Renderer::cleanup()
{
	vk::PipelineSystem::s_inst.cleanup();
	vk::ShaderSystem::s_inst.cleanup();
	vk::VulkanSystem::s_inst.cleanup();
}

}