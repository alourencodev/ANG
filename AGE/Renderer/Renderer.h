#pragma once

#include <Core/SArray.hpp>

#include <AGE/Renderer/Vulkan/VulkanContext.h>
#include <AGE/Renderer/Vulkan/VulkanFrameSync.h>
#include <AGE/Renderer/Vulkan/VulkanSwapchain.h>
#include <AGE/Renderer/Vulkan/VulkanRenderTarget.h>



struct GLFWwindow;

namespace age
{

class Renderer
{
	static const u8 k_maxFramesInFlight = 3;

public:
	static Renderer s_inst;

	void init(GLFWwindow *window);
	void draw();
	void cleanup();

	void onWindowResize(GLFWwindow *window, int width, int height);

private:
	vk::Context _context;
	vk::Swapchain _swapchain;
	vk::RenderTarget _renderTarget;
	SArray<vk::FrameSync, k_maxFramesInFlight> _frameSyncArray;
	u8 _currentFrame = 0;
	math::sizei _windowSize;

	void submit(VkCommandBuffer &commandBuffer);
	void present(u32 imageIndex);
	void reacreateSwapchain();
};

}
