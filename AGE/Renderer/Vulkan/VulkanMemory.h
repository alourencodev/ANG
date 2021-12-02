#pragma once

#include <vulkan/vulkan.h>


namespace age::vk
{
	
struct Buffer
{
	VkBuffer buffer;
	VkDeviceMemory memory;
};

Buffer allocBuffer(VkPhysicalDevice physicalDevice, VkDevice device, size_t size);
void freeBuffer(VkDevice device, const Buffer &buffer);

}
