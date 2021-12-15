
#include <AGE/Renderer/Vulkan/VulkanContext.h>
#include <AGE/Renderer/Vulkan/VulkanFrameSync.h>
#include <AGE/Renderer/Vulkan/VulkanUtils.h>


namespace age::vk
{

FrameSync createFrameSync(const Context &context, VkAllocationCallbacks *allocator)
{
	FrameSync frameSync = {};
	frameSync.allocator = allocator;

	{	// Create Semaphores
		VkSemaphoreCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		AGE_VK_CHECK(vkCreateSemaphore(context.device, &createInfo, frameSync.allocator, &frameSync.imageAvailableSemaphore));
		AGE_VK_CHECK(vkCreateSemaphore(context.device, &createInfo, frameSync.allocator, &frameSync.renderFinishedSemaphore));
	}


	{	// Create Fence
		VkFenceCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		createInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		AGE_VK_CHECK(vkCreateFence(context.device, &createInfo, frameSync.allocator, &frameSync.inFlightFence));
	}

	return frameSync;
}



void destroyFrameSync(const Context &context, FrameSync &frameSync)
{
	vkDestroyFence(context.device, frameSync.inFlightFence, frameSync.allocator);

	vkDestroySemaphore(context.device, frameSync.renderFinishedSemaphore, frameSync.allocator);
	vkDestroySemaphore(context.device, frameSync.imageAvailableSemaphore, frameSync.allocator);
}

}	// namesapce age::vk