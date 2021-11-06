#include "AGE/Renderer/Vulkan/VulkanUtils.h"

#include <Core/Attributes.hpp>

namespace age::vk
{

#define AGE_VK_UTILS_CASE_TO_STR(C)  \
case C:                 \
  return #C;			\

std::string parseVulkanError(VkResult result)
{
    switch (result) {
        AGE_VK_UTILS_CASE_TO_STR(VK_NOT_READY);
        AGE_VK_UTILS_CASE_TO_STR(VK_TIMEOUT);
        AGE_VK_UTILS_CASE_TO_STR(VK_EVENT_SET);
        AGE_VK_UTILS_CASE_TO_STR(VK_EVENT_RESET);
        AGE_VK_UTILS_CASE_TO_STR(VK_INCOMPLETE);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_OUT_OF_HOST_MEMORY);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_OUT_OF_DEVICE_MEMORY);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_INITIALIZATION_FAILED);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_DEVICE_LOST);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_MEMORY_MAP_FAILED);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_LAYER_NOT_PRESENT);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_EXTENSION_NOT_PRESENT);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_FEATURE_NOT_PRESENT);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_INCOMPATIBLE_DRIVER);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_TOO_MANY_OBJECTS);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_FORMAT_NOT_SUPPORTED);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_FRAGMENTED_POOL);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_SURFACE_LOST_KHR);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
        AGE_VK_UTILS_CASE_TO_STR(VK_SUBOPTIMAL_KHR);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_OUT_OF_DATE_KHR);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
        AGE_VK_UTILS_CASE_TO_STR(VK_ERROR_VALIDATION_FAILED_EXT);

        default:
            return "UNKOWN";
    }
}
#undef AGE_VK_UTILS_CASE_TO_STR

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
											 VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
											 const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
											 void */*userData*/)
{
	static const char *k_vulkanTag = "Vulkan";
	using VkMessageSeverityFlag = VkDebugUtilsMessageSeverityFlagBitsEXT;

	if (messageSeverity >= VkMessageSeverityFlag::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		age_error(k_vulkanTag, callbackData->pMessage);
	} else if (messageSeverity >= VkMessageSeverityFlag::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		age_warning(k_vulkanTag, callbackData->pMessage);
	} else {
		age_log(k_vulkanTag, callbackData->pMessage);
	}

	return VK_FALSE;
}

}