#pragma once

#include <vulkan/vulkan.h>


namespace age::vk
{

struct Context;
struct Buffer;
	

Buffer allocBuffer(const Context &context, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
void copyToBuffer(const Context &context, Buffer &buffer, const void *srcData, size_t size);
void copyBuffer(const Context &context, const Buffer &src, Buffer &dst, VkDeviceSize size);
void freeBuffer(const Context &context, const Buffer &buffer);

}
