#pragma once

#include <vulkan/vulkan.h>

#include <Core/Attributes.hpp>
#include <Core/BitField.hpp>
#include <Core/BuildScheme.hpp>
#include <Core/DArray.hpp>
#include <Core/SArray.hpp>

#include "AGE/Renderer/Vulkan/VulkanPipelineSystem.h"

struct GLFWwindow;

namespace age::vk
{

class VulkanSystem;

// If a new family is added to the enum, it is also necessary to check if the physical device supports it.
enum class e_QueueFamily : u8
{
	Graphics,
	Presentation,

	Count
};

using CommandBuffers = DArray<VkCommandBuffer>;

class VulkanSystem
{
public:
	struct SwapchainData
	{
		DArray<VkImage> images;
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkExtent2D extent = {0, 0};
	};

	static VulkanSystem s_inst;

	void init(GLFWwindow *window);
	void cleanup();

	void draw(const CommandBuffers &commandBuffers);

	CommandBuffers allocDrawCommandBuffer(PipelineHandle pipelineHandle) const;
	void freeDrawCommandBuffers(CommandBuffers &commandBuffers) const;

	VkDevice device() const { return _device; }
	VkRenderPass renderPass() const { return _renderPass; }
	SwapchainData swapchainData() const { return _swapchainData; }

private:
	using QueueArray = SArray<VkQueue, static_cast<u32>(e_QueueFamily::Count)>;

	SwapchainData _swapchainData;
	DArray<VkImageView> _imageViews;
	DArray<VkFramebuffer> _framebuffers;
	QueueArray _queueArray;

	VkInstance _instance = VK_NULL_HANDLE;
	VkSurfaceKHR _surface = VK_NULL_HANDLE;
	VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
	VkDevice _device = VK_NULL_HANDLE;
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
	VkRenderPass	 _renderPass;
	VkCommandPool _graphicsCommandPool;

	VkSemaphore _imageAvailableSemaphore;
	VkSemaphore _renderFinishedSemaphore;

	u8 _currentFrame = 0;

#ifdef AGE_RELEASE_DBG_INFO
	VkDebugUtilsMessengerEXT _debugMessenger;
#endif
};

}
