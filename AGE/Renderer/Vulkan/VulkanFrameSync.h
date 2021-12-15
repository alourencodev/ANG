#pragma once

#include <vulkan/vulkan.h>


namespace age::vk
{

struct Context;



struct FrameSync
{
	VkAllocationCallbacks *allocator = nullptr;
	VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
	VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
	VkFence inFlightFence = VK_NULL_HANDLE;
};



FrameSync createFrameSync(const Context &context, VkAllocationCallbacks *allocator);
void destroyFrameSync(const Context &context, FrameSync &frameSync);

}
