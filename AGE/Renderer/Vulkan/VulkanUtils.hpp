#pragma once

#include <vulkan/vulkan.h>
#include <Core/Log/Log.h>


#define AGE_VK_CHECK(exp)														\
{																				\
	VkResult vkCheckResult = exp;												\
	if (vkCheckResult != VK_SUCCESS)											\
		g_error("Vulkan", "Vulkan check failed for expression (" #exp ")")		\
}
