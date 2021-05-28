#pragma once

#include <vulkan/vulkan.h>

#include <Core/Attributes.hpp>
#include <Core/BitField.hpp>
#include <Core/BuildScheme.hpp>
#include <Core/DArray.hpp>
#include <Core/SArray.hpp>


namespace age::vk
{

class VulkanSystem;

enum class e_QueueFamily : u8
{
	Graphics,

	Count
};

class VulkanSystem
{
public:
	static VulkanSystem &get() { static VulkanSystem instance; return instance; } ;
	
	void init();
	void cleanup();

private:
	VkInstance _instance = VK_NULL_HANDLE;
	VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
	VkDevice _device = VK_NULL_HANDLE;

#ifdef _RELEASE_SYMB
	VkDebugUtilsMessengerEXT _debugMessenger;
#endif
};

}
