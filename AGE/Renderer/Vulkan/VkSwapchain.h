#pragma once

#include <vulkan/vulkan.h>

#include <Core/DArray.hpp>
#include <Core/Math/Vec2.hpp>


namespace age::vk
{

struct Context;



struct Swapchain
{
	VkAllocationCallbacks *allocator = nullptr;
	DArray<VkImage> images = {};
	DArray<VkImageView> imageViews = {};
	DArray<VkFence> imageInFlightFences = {};
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkFormat format = VK_FORMAT_UNDEFINED;
	VkExtent2D extent = {0, 0};
};



struct SwapchainCreateInfo
{
	math::sizei framebufferSize = {0, 0};
	VkAllocationCallbacks *allocator = nullptr;
};



Swapchain createSwapchain(const Context &context, const SwapchainCreateInfo &info);
void destroySwapchain(const Context &context, Swapchain &swpchain);

}	// namespace age::vk
