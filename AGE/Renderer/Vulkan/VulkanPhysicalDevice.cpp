
#include <AGE/Renderer/Vulkan/VulkanPhysicalDevice.h>

#include <vulkan/vulkan.h>

#include <Core/BitField.hpp>
#include <Core/DArray.hpp>

#include <AGE/Renderer/Vulkan/VulkanContext.h>
#include <AGE/Renderer/Vulkan/VulkanUtils.h>


namespace age::vk
{

constexpr const char k_tag[] = "VulkanBootstrap";

/**
@brief	Helper structure that allows to quick check queue existence by queue family.
**/
struct QueueIndexBitMap
{
	QueueIndexArray indices = {};
	BitField map;
};



QueueIndexBitMap getDeviceQueueIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

	DArray<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	queueFamilies.addEmpty(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

	QueueIndexBitMap queueIndices;
	for (u32 i = 0; i < queueFamilies.count(); i++) {

		// Graphics Queue
		{
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				const u8 graphicsIndex = static_cast<u8>(EQueueFamily::Graphics);
				queueIndices.map.set(graphicsIndex);
				queueIndices.indices[graphicsIndex] = i;
			}
		}

		// Presentation Queue
		{
			VkBool32 supportsPresentation = false;
			AGE_VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresentation));

			if (supportsPresentation) {
				const u8 presentationIndex = static_cast<u8>(EQueueFamily::Presentation);
				queueIndices.map.set(presentationIndex);
				queueIndices.indices[presentationIndex] = i;
			}
		}

		// Transfer Queue
		{
			const VkQueueFlags queueFlags = queueFamilies[i].queueFlags;
			if ((queueFlags & VK_QUEUE_TRANSFER_BIT) && !(queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queueFlags & VK_QUEUE_COMPUTE_BIT)) {
				const u8 transferIndex = static_cast<u8>(EQueueFamily::Transfer);
				queueIndices.map.set(transferIndex);
				queueIndices.indices[transferIndex] = i;
			}
		}
	}

	return queueIndices;
}



SurfaceData getSurfaceData(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
{
	SurfaceData details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

	u32 formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	if (formatCount > 0) {
		details.formats.reserve(formatCount);
		details.formats.addEmpty(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
	}

	u32 presentModesCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, nullptr);
	if (presentModesCount > 0) {
		details.presentMode.reserve(presentModesCount);
		details.presentMode.addEmpty(presentModesCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModesCount, details.presentMode.data());
	}

	return details;
}



bool isDeviceCompatible(VkPhysicalDevice physicalDevice, 
						const QueueIndexBitMap &queueIndexBitMap, 
						const SurfaceData &swapChainDetails,
						const DArray<const char *> &extensions)
{
	// CheckRequiredQueueFamilies
	if (!queueIndexBitMap.map.isSetBelow(static_cast<u8>(EQueueFamily::Count)))
		return false;

	{	// CheckRequiredExtensions
		u32 extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

		DArray<VkExtensionProperties> availableExtensions(extensionCount);
		availableExtensions.addEmpty(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

		DArray<const char *> missingRequiredExtensions(extensions);
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

	// CheckSwapChain
	if (swapChainDetails.formats.isEmpty() || swapChainDetails.presentMode.isEmpty())
		return false;

	return true;
}



VkPhysicalDevice pickPhysicalDevice(const DArray<VkPhysicalDevice> &candidates, 
									VkSurfaceKHR surface, 
									const DArray<const char *> extensions,  
									QueueIndexArray &o_queueIndices)
{
	// Pick the first dedicated GPU. 
	// If there is no dedicated GPU, fallback to the first candidate.

	for (VkPhysicalDevice candidate : candidates) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(candidate, &properties);
		
		QueueIndexBitMap queueIndexBitMap = getDeviceQueueIndices(candidate, surface);
		SurfaceData swapChainDetails = getSurfaceData(candidate, surface);
		if (!isDeviceCompatible(candidate, queueIndexBitMap, swapChainDetails, extensions))
			continue;

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			age_log(k_tag, "Picked %s as a physical device", properties.deviceName);
			o_queueIndices = queueIndexBitMap.indices;

			return candidate;
		}
	}

	return VK_NULL_HANDLE;
}



void selectPhysicalDevice(Context &context, const DArray<const char *> &extensions)
{
	u32 deviceCount = 0;
	vkEnumeratePhysicalDevices(context.instance, &deviceCount, nullptr);
	age_assertFatal(deviceCount > 0, "Unable to find physical devices with Vulkan support.");

	DArray<VkPhysicalDevice> physicalDevices(deviceCount);
	physicalDevices.addEmpty(deviceCount);
	vkEnumeratePhysicalDevices(context.instance, &deviceCount, physicalDevices.data());

	context.physicalDevice = pickPhysicalDevice(physicalDevices, 
												context.surface, 
												extensions, 
												context.queueIndices);
	age_assertFatal(context.instance != VK_NULL_HANDLE, "Unable to find a suitable physical device.");
}

}