#pragma once

#include <vulkan/vulkan.h>


namespace age::vk
{

struct Context;
struct Buffer;
	

Buffer allocBuffer(const Context &context, size_t size);
void freeBuffer(const Context &context, const Buffer &buffer);

}
