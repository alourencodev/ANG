#pragma once

#include <vulkan/vulkan.h>

#include <Core/BuildScheme.hpp>


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

	VkInstance _instance;

#ifdef _RELEASE_SYMB
	VkDebugUtilsMessengerEXT _debugMessenger;
#endif
};

}
