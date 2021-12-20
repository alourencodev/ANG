#pragma once

#include <vulkan/vulkan.h>

#include <Core/DArray.hpp>
#include <Core/SArray.hpp>



namespace age::vk
{

struct Context;

void selectPhysicalDevice(Context &context, const DArray<const char *> &requiredExtensions);

}
