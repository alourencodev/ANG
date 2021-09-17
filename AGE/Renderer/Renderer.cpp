#include <AGE/Renderer/Renderer.h>

#include <AGE/Renderer/Vulkan/VulkanSystem.h>
#include <AGE/Renderer/Vulkan/VulkanShaderSystem.h>

namespace age
{

Renderer Renderer::s_inst = Renderer();

void Renderer::init(GLFWwindow *window)
{
	vk::VulkanSystem::s_inst.init(window);

	// TODO: Remove shader test code
	ShaderHandle vertexShader = vk::ShaderSystem::s_inst.createShader(vk::e_ShaderStage::Vertex, "Shaders/dummy.vert.spv");
	ShaderHandle fragmentShader = vk::ShaderSystem::s_inst.createShader(vk::e_ShaderStage::Fragment, "Shaders/dummy.frag.spv");
}

void Renderer::update()
{
	// TODO
}

void Renderer::cleanup()
{
	vk::ShaderSystem::s_inst.cleanup();
	vk::VulkanSystem::s_inst.cleanup();
}

}