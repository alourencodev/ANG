#pragma once

#include <vulkan/vulkan.h>

#include <Core/DArray.hpp>
#include <Core/Math/Vec3.hpp>


namespace age::vk
{

struct Context;
struct Swapchain;



struct RenderTarget
{
	VkAllocationCallbacks *allocator = nullptr;
	DArray<VkFramebuffer> framebuffers = {};
	VkRenderPass renderPass = VK_NULL_HANDLE;
};



RenderTarget createRenderTarget(const Context &context, const Swapchain &swapchain, VkAllocationCallbacks *allocator);
void destroyRenderTarget(const Context &context, RenderTarget &renderTarget);

}	// namespace age::vk
