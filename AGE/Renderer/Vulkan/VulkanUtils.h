#pragma once

#include <string>

#include <vulkan/vulkan.h>

#include <Core/Log/Log.h>
#include <Core/Types.hpp>


namespace age::vk
{

u32 findMemoryType(VkPhysicalDevice physicalDevice, u32 typeFilter, VkMemoryPropertyFlags properties);

std::string parseVulkanError(VkResult result);

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
											 VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
											 const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
											 void */*userData*/);

}

#define age_vulkanError(result) logVulkanError(__FILE__, __LINE__, result)

#define AGE_VK_CHECK(exp)																												\
{																																		\
	VkResult vkCheckResult = exp;																										\
	if (vkCheckResult != VK_SUCCESS)																									\
		age_error("Vulkan", "Vulkan check failed for expression (" #exp ") with error %s", parseVulkanError(vkCheckResult).c_str());	\
}

#define AGE_VK_GET_COMMAND(instance, command_name) reinterpret_cast<PFN_##command_name>(vkGetInstanceProcAddr(instance, #command_name))
