#pragma once

// TODO: After testing, check if this include is necessary
#include <AGE/Renderer/Vulkan/VulkanSystem.h>

struct GLFWwindow;

namespace age
{

class Renderer
{
public:
	static Renderer s_inst;

	void init(GLFWwindow *window);
	void update();
	void cleanup();

	void onWindowResize(GLFWwindow *window, int width, int height);

private:
	// TODO: Remove after testing
	vk::CommandBufferArray _testCommandBuffers;
};

}
