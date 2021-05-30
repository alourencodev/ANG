#include "VulkanSystem.h"

#include <cstring>

#include <Core/DArray.hpp>
#include <Core/Log/Assert.hpp>
#include <Core/Log/Log.h>
#include <Core/Meta.hpp>
#include <Core/Types.hpp>
#include <Core/Math/Math.hpp>

#include "AGE/Vendor/GLFW.hpp"
#include "AGE/Renderer/Vulkan/VulkanUtils.h"


namespace age::vk
{

constexpr char k_tag[] = "VulkanSystem";

struct QueueIndices
{
	using QueueIndexArray = SArray<u32, static_cast<u8>(e_QueueFamily::Count)>;

	QueueIndexArray indices = {};
	BitField indexMap;
};

static const SArray<const char *, 1> k_requiredExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef _RELEASE_SYMB
// TODO: Check what other validation layers can be useful to us
static const SArray<const char *, 1> k_validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

static const SArray<const char *, 1> k_debugExtensions = {
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

constexpr byte getRequiredQueueFamilies()
{
	byte field = 0;
	for (int i = 0; i < static_cast<u8>(e_QueueFamily::Count); i++)
		field |= 1 << i;
	
	return field;
}

QueueIndices getDeviceQueueIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	// TODO: Use stack allocator here
	DArray<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	queueFamilies.addEmpty(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	QueueIndices queueIndices;
	for (u32 i = 0; i < queueFamilies.count(); i++) {

		// Graphics Queue
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				const u8 graphicsIndex = static_cast<u8>(e_QueueFamily::Graphics);
				queueIndices.indexMap.set(graphicsIndex);
				queueIndices.indices[graphicsIndex] = i;
			}
		}

		// Presentation Queue
		{
			VkBool32 supportsPresentation = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresentation);

			if (supportsPresentation) {
				const u8 presentationIndex = static_cast<u8>(e_QueueFamily::Presentation);
				queueIndices.indexMap.set(presentationIndex);
				queueIndices.indices[presentationIndex] = i;
			}
		}
	}

	return queueIndices;
}

bool isDeviceCompatible(VkPhysicalDevice physicalDevice, const QueueIndices &queueIndices)
{
	{	// CheckRequiredQueueFamilies
		if (!queueIndices.indexMap.isSet(BitField(getRequiredQueueFamilies())))
			return false;
	}
	
	{	// CheckRequiredExtensions
		u32 extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

		// TODO: Use Stack Allocator
		DArray<VkExtensionProperties> availableExtensions(extensionCount);
		availableExtensions.addEmpty(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		DArray<const char *> missingRequiredExtensions(k_requiredExtensions);
		for (const auto &extension : availableExtensions) {
			for (int i = 0; i < missingRequiredExtensions.count(); i++) {
				if (strcmp(missingRequiredExtensions[i], extension.extensionName) == 0) {
					missingRequiredExtensions.swapPopIndex(i);
					break;
				}
			}
		}

		if (!missingRequiredExtensions.isEmpty())
			return false;
	}

	return true;
}

VkPhysicalDevice pickPhysicalDevice(const DArray<VkPhysicalDevice> &candidates, QueueIndices &out_queueIndices, VkSurfaceKHR surface)
{
	// TODO https://trello.com/c/FZ8pfoMI
	// Currently we just pick the first dedicated GPU. 
	// If there is no dedicated GPU we fallback to the first candidate.
	// This is a very naive way to pick a GPU, so a better way should be implemented in the future.

	for (VkPhysicalDevice candidate : candidates) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(candidate, &properties);
		
		QueueIndices queueIndices = getDeviceQueueIndices(candidate, surface);
		if (!isDeviceCompatible(candidate, queueIndices))
			continue;

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			g_log(k_tag, "Picked %s as a physical device", properties.deviceName);
			out_queueIndices = queueIndices;

			return candidate;
		}
	}

	return VK_NULL_HANDLE;
}

void VulkanSystem::init(GLFWwindow *window)
{
	// TODO: Remove
	logger::enable(k_tag);

	DArray<const char *> extensions;
	QueueIndices queueIndices;

	{	// GetRequiredExtensions
		u32 glfwExtensionCount = 0;
		const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

#ifdef _RELEASE_SYMB
		extensions.reserve(glfwExtensionCount + k_debugExtensions.size);
		extensions.add(k_debugExtensions);
#else
		extensions.reserve(glfwExtensionCount);
#endif

		extensions.add(glfwExtensions, glfwExtensionCount);
	}

#ifdef _RELEASE_SYMB
	{ 	// CheckValidationLayersAvailability
		u32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		DArray<VkLayerProperties> availableLayerProperties(layerCount);
		availableLayerProperties.addEmpty(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayerProperties.data());

		for (const char *layer : k_validationLayers) {
			bool didFindLayer = false;

			for (const auto &property : availableLayerProperties) {
				if (strcmp(layer, property.layerName) == 0) {
					didFindLayer = true;
					break;
				}
			}

			g_assertFatal(didFindLayer, "Requested validation layer(%s) is not available.", layer);
		}
	}
#endif

	{	// CreateInstance
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

#ifdef _RELEASE_SYMB
		createInfo.enabledLayerCount = static_cast<u32>(k_validationLayers.size);
		createInfo.ppEnabledLayerNames = k_validationLayers.data();
		createInfo.enabledExtensionCount = static_cast<u32>(extensions.count());
		createInfo.ppEnabledExtensionNames = extensions.data();
#else
		createInfo.enabledLayerCount = 0;
		createInfo.enabledExtensionCount = 0;
#endif

		AGE_VK_CHECK(vkCreateInstance(&createInfo, nullptr, &_instance));
		g_log(k_tag, "Create Vulkan Instance.");
	}

	{	// CreateWindowSurface
		AGE_VK_CHECK(glfwCreateWindowSurface(_instance, window, nullptr, &_surface));
		g_log(k_tag, "Create Window Surface.");
	}

#ifdef _RELEASE_SYMB
	{	// CreateDebugMessenger
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
			g_error(k_tag, "Unable to create DebugUtilsMessenger due to extension not being present.");
		}
	}
#endif

	{	// PickPhysicalDevice
		u32 deviceCount = 0;
		vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
		g_assertFatal(deviceCount > 0, "Unable to find physical devices with Vulkan support.");

		// TODO: Use stack allocator here
		DArray<VkPhysicalDevice> physicalDevices(deviceCount);
		physicalDevices.addEmpty(deviceCount);
		vkEnumeratePhysicalDevices(_instance, &deviceCount, physicalDevices.data());

		_physicalDevice = pickPhysicalDevice(physicalDevices, queueIndices, _surface);
		g_assertFatal(_physicalDevice != VK_NULL_HANDLE, "Unable to find a suitable physical device.");
	}

	{	// CreateLogicalDevice
		// TOOD: Use Stack Allocator
		DArray<VkDeviceQueueCreateInfo> queueCreateInfoArray(static_cast<u32>(e_QueueFamily::Count));

		float queuePriority = 1.0f;
		for (int i = 0; i < static_cast<u32>(e_QueueFamily::Count); i++) {
			VkDeviceQueueCreateInfo queueCreateInfo;
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pNext = nullptr;
			queueCreateInfo.flags = 0;
			queueCreateInfo.queueFamilyIndex = queueIndices.indices[i];
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfoArray.add(queueCreateInfo);
		}

		VkDeviceCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.pQueueCreateInfos = queueCreateInfoArray.data();
		createInfo.queueCreateInfoCount = queueCreateInfoArray.count();
		createInfo.pEnabledFeatures = nullptr;	// TODO: Assign
		createInfo.enabledExtensionCount = k_requiredExtensions.size;
		createInfo.ppEnabledExtensionNames = k_requiredExtensions.data();

		// For older versions on Vulkan, it might be necessary to also set the validation layers here.
		// Since this is not necessary at the time of this implementation, let's skip that.
		// TODO: Investigate how this is really currently working as it seems to be generating weird errors.
		createInfo.enabledLayerCount = 0;
		
		AGE_VK_CHECK(vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device));
		g_log(k_tag, "Vulkan device created.");
	}

	{	// StoreQueueHandles
		for (int i = 0; i < static_cast<u8>(e_QueueFamily::Count); i++)
			vkGetDeviceQueue(_device, queueIndices.indices[i], 0, &_queueArray[i]);
	}
}

void VulkanSystem::cleanup()
{
	vkDestroyDevice(_device, nullptr);

#ifdef _RELEASE_SYMB
	{	// DestroyDebugUtilsMessenger
		auto f_destroyDebugUtilsMessengerEXT = AGE_VK_GET_COMMAND(_instance, vkDestroyDebugUtilsMessengerEXT);
		if (f_destroyDebugUtilsMessengerEXT != nullptr)
			f_destroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
	}
#endif

	vkDestroySurfaceKHR(_instance, _surface, nullptr);
	vkDestroyInstance(_instance, nullptr);
}

}