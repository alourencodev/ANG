
#include <Core/Log/Log.h>

#include <AGE/Renderer/Vulkan/VulkanData.h>
#include <AGE/Renderer/Vulkan/VulkanMemory.h>
#include <AGE/Renderer/Vulkan/VulkanUtils.h>



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



Buffer allocBuffer(const Context &context, size_t size)
{
	Buffer buffer;

	{	// Create Buffer
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.flags = 0;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		AGE_VK_CHECK(vkCreateBuffer(context.device, &bufferInfo, nullptr, &buffer.buffer));
	}


	{	// Allocate Buffer
		VkMemoryRequirements memoryRequirements;
		vkGetBufferMemoryRequirements(context.device, buffer.buffer, &memoryRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memoryRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(context.physicalDevice, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		AGE_VK_CHECK(vkAllocateMemory(context.device, &allocInfo, nullptr, &buffer.memory));

		vkBindBufferMemory(context.device, buffer.buffer, buffer.memory, 0);
	}

	return buffer;
}



void freeBuffer(const Context &context, const Buffer &buffer)
{
	vkDestroyBuffer(context.device, buffer.buffer, nullptr);
	vkFreeMemory(context.device, buffer.memory, nullptr);
}

}