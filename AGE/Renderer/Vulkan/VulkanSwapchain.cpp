#include <AGE/Renderer/Vulkan/VulkanSwapchain.h>

#include <AGE/Renderer/Vulkan/VulkanContext.h>
#include <AGE/Renderer/Vulkan/VulkanUtils.h>


namespace age::vk
{

constexpr const char k_tag[] = "VulkanBootstrap";



struct SurfaceData
{
	VkSurfaceCapabilitiesKHR capabilities;
	DArray<VkSurfaceFormatKHR> formats;
	DArray<VkPresentModeKHR> presentMode;
};



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



void createSwapchainInternal(const Context &context, const SwapchainCreateInfo &info, Swapchain &swapchain)
{
	SurfaceData surfaceData = getSurfaceData(context.physicalDevice, context.surface);
	const VkSurfaceCapabilitiesKHR &capabilities = surfaceData.capabilities;

	VkColorSpaceKHR colorSpace;
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;


	{	// Choose Format
		VkSurfaceFormatKHR surfaceFormat = surfaceData.formats[0];

		for (const auto &availableFormat : surfaceData.formats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				surfaceFormat = availableFormat;
				break;
			}
		}
		
		colorSpace = surfaceFormat.colorSpace;
		swapchain.format = surfaceFormat.format;
	}


	{	// Choose PresentMode
		for (const auto &availablePresentMode : surfaceData.presentMode) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
				presentMode = availablePresentMode;
		}
	}


	{	// Choose Extension
		VkExtent2D &extent = swapchain.extent;
		extent = {info.framebufferSize.w, info.framebufferSize.h};

		extent.width = math::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		extent.height = math::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	}


	u32 minImageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && capabilities.maxImageCount < minImageCount) {
		age_warning(k_tag, "Trying to have more swapchain images than what is allowed. SwapChain images count will be clamped to the allowed maximum.");
		minImageCount = capabilities.maxImageCount;
	}

	age_log(k_tag, "Using %d swapchain images.", minImageCount);

	VkSwapchainCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.surface = context.surface;
	createInfo.minImageCount = minImageCount;
	createInfo.imageFormat = swapchain.format;
	createInfo.imageColorSpace = colorSpace;
	createInfo.imageExtent = swapchain.extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	u32 presentationQueueIndex = context.queueIndices[static_cast<u32>(EQueueFamily::Presentation)];
	u32 graphicsQueueIndex = context.queueIndices[static_cast<u32>(EQueueFamily::Graphics)];
	if (presentationQueueIndex != graphicsQueueIndex) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = static_cast<u32>(context.queueIndices.size());
		createInfo.pQueueFamilyIndices = context.queueIndices.data();
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}


	{	// Store Image Handles
		u32 imageCount = 0;
		vkGetSwapchainImagesKHR(context.device, swapchain.swapchain, &imageCount, nullptr);

		swapchain.images.reserveWithEmpty(imageCount);
		vkGetSwapchainImagesKHR(context.device, swapchain.swapchain, &imageCount, swapchain.images.data());
	}


	AGE_VK_CHECK(vkCreateSwapchainKHR(context.device, &createInfo, swapchain.allocator, &swapchain.swapchain));
	age_log(k_tag, "Swapchain created.");
}



void createImageViews(const Context &context, Swapchain &swapchain)
{
	const auto &images = swapchain.images;
	swapchain.imageViews.reserveWithEmpty(images.count());

	for (int i = 0; i < images.count(); i++) {
		VkImageViewCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.image = images[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapchain.format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		AGE_VK_CHECK(vkCreateImageView(context.device, &createInfo, swapchain.allocator, &swapchain.imageViews[i]))
	}

	age_log(k_tag, "Created %d image views", swapchain.imageViews.count());
}



Swapchain createSwapchain(const Context &context, const SwapchainCreateInfo &info)
{
	Swapchain swapchain = {};
	swapchain.allocator = info.allocator;

	createSwapchainInternal(context, info, swapchain);
	createImageViews(context, swapchain);

	return swapchain;
}



void destroySwapchain(const Context &context, Swapchain &swapchain)
{
	for (auto imageView : swapchain.imageViews)
		vkDestroyImageView(context.device, imageView, swapchain.allocator);
	swapchain.imageViews.clear();

	vkDestroySwapchainKHR(context.device, swapchain.swapchain, swapchain.allocator);
}

}	// namespace age::vk