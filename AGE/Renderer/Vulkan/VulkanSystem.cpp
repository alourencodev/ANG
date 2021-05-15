#include "VulkanSystem.h"

#include <cstring>

#include <Core/DArray.hpp>
#include <Core/Meta.hpp>
#include <Core/Types.hpp>

#include "AGE/Vendor/GLFW.hpp"
#include "AGE/Renderer/Vulkan/VulkanUtils.h"


namespace age
{

constexpr char k_tag[] = "VulkanSystem";

#ifdef _RELEASE_SYMB
static const DArray<const char *> k_validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

static const DArray<const char *> k_debugExtensions = {
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
													VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
													const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
													void */*userData*/)
{
	static const char *k_vulkanTag = "Vulkan";
	using VkMessageSeverityFlag = VkDebugUtilsMessageSeverityFlagBitsEXT;

	if (messageSeverity >= VkMessageSeverityFlag::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		g_error(k_vulkanTag, callbackData->pMessage);
	} else if (messageSeverity >= VkMessageSeverityFlag::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		g_warning(k_vulkanTag, callbackData->pMessage);
	} else {
		g_log(k_vulkanTag, callbackData->pMessage);
	}

	return VK_FALSE;
}
#endif

void VulkanSystem::init()
{
	constexpr bool isValidationEnabled = meta::isReleaseSymbBuild::value;

	DArray<const char *> extensions;
	// GetRequiredExtensions
	{
		u32 glfwExtensionCount = 0;
		const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		if constexpr (isValidationEnabled) {
			extensions.reserve(glfwExtensionCount + k_debugExtensions.count());
			extensions.add(k_debugExtensions);
		} else {
			extensions.reserve(glfwExtensionCount);
		}

		extensions.add(glfwExtensions, glfwExtensionCount);
	}

	// Check if validation layers are available
	if constexpr(isValidationEnabled)
	{	
		u32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		DArray<VkLayerProperties> availableLayerProperties(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayerProperties.data());

		for (const char *layer : k_validationLayers) {
			bool didFindLayer = false;

			for (const auto &property : availableLayerProperties) {
				if (strcmp(layer, property.layerName) == 0) {
					didFindLayer = true;
					break;
				}
			}

			g_assertFatal(k_tag, "Requested validation layer(%s) is not available", layer);
		}
	}

	{	// Create Instance
		VkApplicationInfo appInfo;
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pApplicationName = "TEST Name";				// TODO
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);	// TODO 
		appInfo.pEngineName = "AGEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.pApplicationInfo = &appInfo;

		if constexpr (isValidationEnabled) {
			createInfo.enabledLayerCount = static_cast<u32>(k_validationLayers.count());
			createInfo.ppEnabledLayerNames = k_validationLayers.data();
			createInfo.enabledExtensionCount = static_cast<u32>(extensions.count());
			createInfo.ppEnabledExtensionNames = extensions.data();

		} else {
			createInfo.enabledLayerCount = 0;
		}

		AGE_VK_CHECK(vkCreateInstance(&createInfo, nullptr, &_instance));
	}

#ifdef _RELEASE_SYMB
	{	// Create Debug Messenger
		using VkMessageSeverityFlag = VkDebugUtilsMessageSeverityFlagBitsEXT;
		using VkMessageTypeFlag = VkDebugUtilsMessageTypeFlagBitsEXT;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.messageSeverity = VkMessageSeverityFlag::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
									 VkMessageSeverityFlag::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
									 VkMessageSeverityFlag::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
									 VkMessageSeverityFlag::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
		createInfo.messageType = VkMessageTypeFlag::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
								 VkMessageTypeFlag::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
								 VkMessageTypeFlag::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;

		auto f_createDebugUtilsMessengerEXT = AGE_VK_GET_COMMAND(_instance, vkCreateDebugUtilsMessengerEXT);
		if (f_createDebugUtilsMessengerEXT != nullptr) {
			AGE_VK_CHECK(f_createDebugUtilsMessengerEXT(_instance, &createInfo, nullptr, &_debugMessenger));
		} else {
			g_error(k_tag, "Unable to create DebugUtilsMessenger due to extension not being present");
		}
	}
#endif

	{	// Create Physical Device
		u32 deviceCount = 0;
		vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
		g_assertFatal(deviceCount > 0, "Unable to find physical devices with Vulkan support.");

		DArray<VkPhysicalDevice> physicalDevices(deviceCount);
		vkEnumeratePhysicalDevices(_instance, &deviceCount, physicalDevices.data());

		_physicalDevice = pickPhysicalDevice(physicalDevices);
		g_assertFatal(_physicalDevice != VK_NULL_HANDLE, "Unable to find a suitable physical device.");
	}
}

void VulkanSystem::cleanup()
{
	{	// Destroy Debug Utils Messenger
		auto f_destroyDebugUtilsMessengerEXT = AGE_VK_GET_COMMAND(_instance, vkDestroyDebugUtilsMessengerEXT);
		if (f_destroyDebugUtilsMessengerEXT != nullptr)
			f_destroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
	}

	vkDestroyInstance(_instance, nullptr);
}

VkPhysicalDevice VulkanSystem::pickPhysicalDevice(const DArray<VkPhysicalDevice> &candidates) const
{
	// TODO https://trello.com/c/FZ8pfoMI
	// Currently we just pick the first dedicated GPU. 
	// If there is no dedicated GPU we fallback to the first candidate.
	// This is a very naive way to pick a GPU, so a better way should be implemented in the future.

	for (VkPhysicalDevice candidate : candidates) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(candidate, &properties);

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			return candidate;
	}

	return candidates[0];
}

}