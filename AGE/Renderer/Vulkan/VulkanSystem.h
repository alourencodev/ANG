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

constexpr u8 k_maxFramesInFlight = 2;

using CommandBufferArray = DArray<VkCommandBuffer>;

// If a new family is added to the enum, it is also necessary to check if the physical device supports it.
enum class e_QueueFamily : u8
{
	Graphics,
	Presentation,

	Count
};

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

	void draw(const CommandBufferArray &commandBuffers);

	CommandBufferArray allocDrawCommandBuffer(PipelineHandle pipelineHandle) const;
	void freeDrawCommandBuffers(CommandBufferArray &commandBuffers) const;
	void createSwapchain(GLFWwindow *window);

	void waitForAllFrames() const;

	VkDevice device() const { return _device; }
	VkRenderPass renderPass() const { return _renderPass; }
	SwapchainData swapchainData() const { return _swapchainData; }

private:
	using QueueArray = SArray<VkQueue, static_cast<u32>(e_QueueFamily::Count)>;

	struct QueueIndices
	{
		using QueueIndexArray = SArray<u32, static_cast<u8>(e_QueueFamily::Count)>;

		QueueIndexArray indices = {};
		BitField indexMap;
	};

	struct SwapChainDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		DArray<VkSurfaceFormatKHR> formats;
		DArray<VkPresentModeKHR> presentMode;
	};

	struct FrameSyncData
	{
		VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
		VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
		VkFence inFlightFence = VK_NULL_HANDLE;
	};


	void createFrameData(FrameSyncData &frameData);
	void destroyFrameData(FrameSyncData &frameData);

	QueueIndices getDeviceQueueIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const;
	SwapChainDetails getSwapChainDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const;
	VkPhysicalDevice pickPhysicalDevice(const DArray<VkPhysicalDevice> &candidates, VkSurfaceKHR surface, QueueIndices &o_queueIndices, SwapChainDetails &o_swapChainDetails) const;
	bool isDeviceCompatible(VkPhysicalDevice physicalDevice, const QueueIndices &queueIndices, const SwapChainDetails &swapChainDetails) const;

	SwapchainData _swapchainData;
	DArray<VkImageView> _imageViews;
	DArray<VkFramebuffer> _framebuffers;
	QueueArray _queueArray;
	QueueIndices _queueIndices;
	SwapChainDetails _swapchainDetails;

	VkInstance _instance = VK_NULL_HANDLE;
	VkSurfaceKHR _surface = VK_NULL_HANDLE;
	VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
	VkDevice _device = VK_NULL_HANDLE;
	VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
	VkRenderPass	 _renderPass;
	VkCommandPool _graphicsCommandPool;

	SArray<FrameSyncData, k_maxFramesInFlight>	_frameSyncData;
	DArray<VkFence> _imageInFlightFences;	// Stores the fences of the frame that used the indexed image

	u8 _currentFrame = 0;

#ifdef AGE_RELEASE_DBG_INFO
	VkDebugUtilsMessengerEXT _debugMessenger;
#endif
};

}
