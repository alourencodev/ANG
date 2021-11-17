#include <AGE/Renderer/Renderer.h>

#include <AGE/Renderer/Vulkan/Vulkan.h>

namespace age
{

Renderer Renderer::s_inst = Renderer();

constexpr char k_tag[] = "Renderer";

vk::ShaderHandle vertexShader;
vk::ShaderHandle fragmentShader;

vk::DrawCommandHandle testDrawCommand;

void Renderer::init(GLFWwindow *window)
{
	vk::init(window);

	vertexShader = vk::createShader(vk::e_ShaderStage::Vertex, "Shaders/dummy.vert.spv");
	fragmentShader = vk::createShader(vk::e_ShaderStage::Fragment, "Shaders/dummy.frag.spv");

	vk::PipelineCreateInfo info = {};
	info.shaders.add(vertexShader);
	info.shaders.add(fragmentShader);
	vk::PipelineHandle pipelineHandle = vk::createPipeline(info);

	testDrawCommand = vk::createDrawCommand(pipelineHandle);
}

void Renderer::update()
{
	vk::draw(testDrawCommand);
}

void Renderer::cleanup()
{
	vk::cleanup();
}

void Renderer::onWindowResize(GLFWwindow *window, int width, int height)
{
	age_log(k_tag, "Window resized to %d x %d.", width, height);
	vk::recreateRenderEnvironment();
}

}