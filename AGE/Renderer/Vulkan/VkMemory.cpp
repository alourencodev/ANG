
#include <Core/Log/Log.h>

#include <AGE/Renderer/Vulkan/VkContext.h>
#include <AGE/Renderer/Vulkan/VkMemory.h>
#include <AGE/Renderer/Vulkan/VkUtils.h>



namespace age::vk
{

constexpr char k_tag[] = "Vulkan";

u32 findMemoryType(VkPhysicalDevice physicalDevice, u32 typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (u32 i = 0; i < memoryProperties.memoryTypeCount; i++) {
        const bool hasSuitableMemTypes = typeFilter & (1 << i);
        const bool hasSuitableProperties = (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties;

        if (hasSuitableMemTypes && hasSuitableProperties)
            return i;
    }

    age_error(k_tag, "Unable to find suitable memory type.");
}



Buffer allocBuffer(const Context &context, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkAllocationCallbacks *allocator /*= nhullptr*/)
{
	Buffer buffer;
	buffer.allocator = allocator;

	{	// Create Buffer
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.flags = 0;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		AGE_VK_CHECK(vkCreateBuffer(context.device, &bufferInfo, buffer.allocator, &buffer.buffer));
	}


	{	// Allocate Buffer
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(context.device, buffer.buffer, &memoryRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(context.physicalDevice, memoryRequirements.memoryTypeBits, properties);

		// TODO: Have a custom allocator
		AGE_VK_CHECK(vkAllocateMemory(context.device, &allocInfo, buffer.allocator, &buffer.memory));

		vkBindBufferMemory(context.device, buffer.buffer, buffer.memory, 0);
	}

	return buffer;
}



void writeToBuffer(const Context &context, const Buffer &buffer, const void *data, size_t size)
{
	void *dstData;
	vkMapMemory(context.device, buffer.memory, 0, size, 0, &dstData);
	memcpy(dstData, data, size);
	vkUnmapMemory(context.device, buffer.memory);
}



void stageBuffer(const Context &context, const Buffer &buffer, const void *srcData, size_t size)
{
	Buffer stagingBuffer = vk::allocBuffer(context, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	writeToBuffer(context, stagingBuffer, srcData, size);

	// Transfer Buffer to GPU
	copyBuffer(context, stagingBuffer, buffer, size);
	freeBuffer(context, stagingBuffer);
}



void copyBuffer(const Context &context, const Buffer &src, const Buffer &dst, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = context.transferCommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	AGE_VK_CHECK(vkAllocateCommandBuffers(context.device, &allocInfo, &commandBuffer));

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	{
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, src.buffer, dst.buffer, 1, &copyRegion);
	}
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkQueue transferQueue = context.queues[static_cast<u8>(EQueueFamily::Transfer)];
	vkQueueSubmit(transferQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(transferQueue);
}



void freeBuffer(const Context &context, const Buffer &buffer)
{
	vkDestroyBuffer(context.device, buffer.buffer, buffer.allocator);
	vkFreeMemory(context.device, buffer.memory, buffer.allocator);
}

}