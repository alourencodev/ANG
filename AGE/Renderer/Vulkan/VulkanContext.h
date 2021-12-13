#pragma once

#include <vulkan/vulkan.h>

#include <Core/BitField.hpp>
#include <Core/DArray.hpp>
#include <Core/Range.hpp>
#include <Core/SArray.hpp>
#include <Core/Version.hpp>



struct GLFWwindow;

namespace age::vk
{

enum class EQueueFamily : u8
{
	Graphics,
	Presentation,
	Transfer,

	Count
};

constexpr size_t k_queueFamilyCount = static_cast<size_t>(EQueueFamily::Count);

using QueueArray = SArray<VkQueue, k_queueFamilyCount>;
using QueueIndexArray = SArray<u32, k_queueFamilyCount>;



struct Context
{
	VkAllocationCallbacks *allocator = nullptr;
	VkInstance instance = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;
	VkCommandPool transferCommandPool = VK_NULL_HANDLE;
	QueueArray queues = {};
	QueueIndexArray queueIndices = {};

#ifdef AGE_DEBUG
	VkDebugUtilsMessengerEXT debugMessenger;
#endif
};



struct ContextCreateInfo
{
	VkAllocationCallbacks *allocator = nullptr;
	GLFWwindow *window;
	const char *appName = nullptr;
	Version appVersion;
	Version engineVersion;
};



Context createContext(const ContextCreateInfo &createInfo);
void destroyContext(Context &context);

}	// namespace age::vk
