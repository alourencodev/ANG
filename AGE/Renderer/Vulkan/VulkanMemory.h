#pragma once

#include <vulkan/vulkan.h>


namespace age::vk
{

struct Context;
struct Buffer;
	

Buffer allocBuffer(const Context &context, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
void writeToBuffer(const Context &context, const Buffer &buffer, const void *data, size_t size);
void stageBuffer(const Context &context, const Buffer &buffer, const void *srcData, size_t size);
void copyBuffer(const Context &context, const Buffer &src, const Buffer &dst, VkDeviceSize size);
void freeBuffer(const Context &context, const Buffer &buffer);

}
