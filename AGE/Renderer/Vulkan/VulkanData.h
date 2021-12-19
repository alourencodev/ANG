#pragma once

#include <vulkan/vulkan.h>

#include <Core/BitField.hpp>
#include <Core/DArray.hpp>
#include <Core/HashMap.hpp>

#include <AGE/Renderer/Vulkan/Vulkan.h>

namespace age::vk
{

// If a new family is added to the enum, it is also necessary to check if the physical device supports it.
enum class e_QueueFamily : u8
{
	Graphics,
	Presentation,
	Transfer,

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



struct SurfaceData_Legacy
{
	VkSurfaceCapabilitiesKHR capabilities;
	DArray<VkSurfaceFormatKHR> formats;
	DArray<VkPresentModeKHR> presentMode;
};



struct Shader
{
	VkShaderModule module;
	VkShaderStageFlagBits stage;
};



/*
* @brief		Base struct for a vulkan Pipeline.
*			Pipelines have the same info, but are created with different parameters.
*			We have multiple sub structs of Pipeline (e.g. MeshPipeline) to keep type safety.
*/
struct Pipeline
{
	VkPipelineLayout layout = {};
	VkViewport viewport = {};
	VkRect2D scissor = {};
	VkPipeline pipeline = VK_NULL_HANDLE;
	PipelineCreateInfo_Legacy createInfo;
};



/*
* @brief A specific pipeline for a 3D mesh
*/
struct MeshPipeline : public Pipeline
{
};



struct DrawCommand
{
	DArray<VkCommandBuffer> buffers;
	MeshPipelineHandle pipeline;
	MeshHandle mesh;
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
	VkCommandPool transferCommandPool = VK_NULL_HANDLE;

#ifdef AGE_DEBUG
	VkDebugUtilsMessengerEXT debugMessenger;
#endif // AGE_DEBUG
};



/**
@brief	The RendenderEnvironment holds the vulkan structures that are directly necessary
		for rendering. It must be recreated if the widow changes (e.g. resize).
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



/**
@brief	Holds every resource that is referenced externally through handles.
**/
struct Resources
{
	DArray<Shader> shaders;
	DArray<MeshPipeline> meshPipelines;
	HashMap<u32, DrawCommand> drawCommandMap;
};



}	// namespace vk
