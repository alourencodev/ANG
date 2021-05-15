#pragma once

#include <vulkan/vulkan.h>

#include <Core/BuildScheme.hpp>
#include <Core/DArray.hpp>


namespace age
{

class VulkanSystem
{
public:
	static VulkanSystem &get() { static VulkanSystem instance; return instance; } ;
	
	void init();
	void cleanup();

private:
	VulkanSystem() = default;
	~VulkanSystem() = default;

	VkPhysicalDevice pickPhysicalDevice(const DArray<VkPhysicalDevice> &candidates) const;

	VkInstance _instance = VK_NULL_HANDLE;
	VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;

#ifdef _RELEASE_SYMB
	VkDebugUtilsMessengerEXT _debugMessenger;
#endif
};

}
