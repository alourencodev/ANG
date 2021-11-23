#include <AGE/Renderer/Renderer.h>

#include <AGE/Renderer/Vulkan/Vulkan.h>

namespace age
{

const DArray<vk::Vertex> vertices = {
    {{0.0f, -0.5f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}},
    {{-0.5f, 0.5f, 0.0f}}
};

Renderer Renderer::s_inst = Renderer();

constexpr char k_tag[] = "Renderer";

vk::ShaderHandle vertexShader;
vk::ShaderHandle fragmentShader;

vk::DrawCommandHandle testDrawCommand;
vk::MeshBufferHandle testMeshBuffer;

void Renderer::init(GLFWwindow *window)
{
	vk::init(window);

	vertexShader = vk::createShader(vk::e_ShaderStage::Vertex, "Shaders/dummy.vert.spv");
	fragmentShader = vk::createShader(vk::e_ShaderStage::Fragment, "Shaders/dummy.frag.spv");

	vk::PipelineCreateInfo info = {};
	info.shaders.add(vertexShader);
	info.shaders.add(fragmentShader);
	vk::PipelineHandle pipelineHandle = vk::createPipeline(info);

	testMeshBuffer = vk::createMeshBuffer(vertices);
	testDrawCommand = vk::createDrawCommand(pipelineHandle, testMeshBuffer);
}

void Renderer::update()
{
	vk::draw(testDrawCommand);
}

void Renderer::cleanup()
{
	vk::waitForFramesToFinish();

	vk::cleanupDrawCommand(testDrawCommand);
	vk::cleanupMeshBuffer(testMeshBuffer);

	vk::cleanup();
}

void Renderer::onWindowResize(GLFWwindow *window, int width, int height)
{
	age_log(k_tag, "Window resized to %d x %d.", width, height);
	vk::recreateRenderEnvironment();
}

}