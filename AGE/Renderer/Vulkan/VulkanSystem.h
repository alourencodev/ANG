#pragma once

#include <vulkan/vulkan.h>

#include <Core/Attributes.hpp>
#include <Core/BitField.hpp>
#include <Core/BuildScheme.hpp>
#include <Core/DArray.hpp>
#include <Core/SArray.hpp>


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

	VkDevice device() const { return _device; }
	VkRenderPass renderPass() const { return _renderPass; }
	SwapchainData swapchainData() const { return _swapchainData; }

	CommandBuffers allocDrawCommandBuffer(VkPipeline pipeline) const;
	void freeDrawCommandBuffers(const CommandBuffers &commandBuffers) const;

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
	VkCommandPool _commandPool;

#ifdef AGE_RELEASE_DBG_INFO
	VkDebugUtilsMessengerEXT _debugMessenger;
#endif
};

}
