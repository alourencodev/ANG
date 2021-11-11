#pragma once

#include <vulkan/vulkan.h>

#include <Core/BitField.hpp>
#include <Core/Darray.hpp>
#include <Core/SArray.hpp>

struct GLFWwindow;

namespace age::vk
{

// If a new family is added to the enum, it is also necessary to check if the physical device supports it.
enum class e_QueueFamily : u8
{
	Graphics,
	Presentation,

	Count
};

using QueueIndexArray = SArray<u32, static_cast<u8>(e_QueueFamily::Count)>;
using QueueArray = SArray<VkQueue, static_cast<u8>(e_QueueFamily::Count)>;



/**
@brief	Helper structure that allows to quick check queue existence by queue family.
**/
struct QueueIndexBitMap
{
	QueueIndexArray indices = {};
	BitField map;
};



struct FrameSyncData
{
	VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
	VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
	VkFence inFlightFence = VK_NULL_HANDLE;
};



struct SurfaceData
{
	VkSurfaceCapabilitiesKHR capabilities;
	DArray<VkSurfaceFormatKHR> formats;
	DArray<VkPresentModeKHR> presentMode;
};



/**
@brief	The Context holds every data necessary to interface with Vulkan. 
		This is data that doesn't change during an entire sesion.
**/
struct Context
{
	QueueIndexArray queueIndices = {};
	QueueArray queues = {};
	VkInstance instance = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;

#ifdef AGE_DEBUG
	VkDebugUtilsMessengerEXT debugMessenger;
#endif // AGE_DEBUG
};



/**
@brief	The RendenderEnvironment holds the vulkan structures that are directly necessary
		for rendering. It must be created if the widow changes (e.g. resize).
**/
struct RenderEnvironment
{
	DArray<VkImage> images = {};
	DArray<VkImageView> imageViews = {};
	DArray<VkFramebuffer> framebuffers = {};
	DArray<FrameSyncData> frameSyncData = {};
	DArray<VkFence> imageInFlightFences = {};
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkExtent2D swapchainExtent = {0, 0};
	VkRenderPass renderPass = VK_NULL_HANDLE;
	u8 currentFrame = 0;
};



// Static Vulkan State
static GLFWwindow *s_window = nullptr;
static Context s_context;
static RenderEnvironment s_environment;

}	// namespace age::vk
