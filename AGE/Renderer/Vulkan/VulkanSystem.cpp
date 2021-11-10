#include "VulkanSystem.h"

#include <cstring>

#include <Core/BuildScheme.hpp>
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

VulkanSystem VulkanSystem::s_inst = VulkanSystem();

static const SArray<const char *, 1> k_requiredExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef AGE_DEBUG
// TODO: Check what other validation layers can be useful to us
static const SArray<const char *, 1> k_validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

static const SArray<const char *, 1> k_debugExtensions = {
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

#endif

VulkanSystem::QueueIndices VulkanSystem::getDeviceQueueIndices(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const
{
	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

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

VulkanSystem::SwapChainDetails VulkanSystem::getSwapChainDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const
{
	SwapChainDetails details;
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

bool VulkanSystem::isDeviceCompatible(VkPhysicalDevice physicalDevice, const QueueIndices &queueIndices, const SwapChainDetails &swapChainDetails) const
{
	// CheckRequiredQueueFamilies
	if (!queueIndices.indexMap.isSetBelow(static_cast<u8>(e_QueueFamily::Count)))
		return false;

	{	// CheckRequiredExtensions
		u32 extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

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

	// CheckSwapChain
	if (swapChainDetails.formats.isEmpty() || swapChainDetails.presentMode.isEmpty())
		return false;

	return true;
}

VkPhysicalDevice VulkanSystem::pickPhysicalDevice(const DArray<VkPhysicalDevice> &candidates, VkSurfaceKHR surface, QueueIndices &o_queueIndices, SwapChainDetails &o_swapChainDetails) const
{
	// TODO https://trello.com/c/FZ8pfoMI
	// Currently we just pick the first dedicated GPU. 
	// If there is no dedicated GPU we fallback to the first candidate.
	// This is a very naive way to pick a GPU, so a better way should be implemented in the future.

	for (VkPhysicalDevice candidate : candidates) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(candidate, &properties);
		
		QueueIndices queueIndices = getDeviceQueueIndices(candidate, surface);
		SwapChainDetails swapChainDetails = getSwapChainDetails(candidate, surface);
		if (!isDeviceCompatible(candidate, queueIndices, swapChainDetails))
			continue;

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			age_log(k_tag, "Picked %s as a physical device", properties.deviceName);
			o_queueIndices = queueIndices;
			o_swapChainDetails = std::move(swapChainDetails);

			return candidate;
		}
	}

	return VK_NULL_HANDLE;
}

void VulkanSystem::init(GLFWwindow *window)
{
	DArray<const char *> extensions;
	QueueIndices queueIndices;

	{	// GetRequiredExtensions
		u32 glfwExtensionCount = 0;
		const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		extensions.reserve(glfwExtensionCount IF_DEBUG(+ k_debugExtensions.size()));

#ifdef AGE_DEBUG
		extensions.add(k_debugExtensions);
#endif
		extensions.add(glfwExtensions, glfwExtensionCount);
	}

#ifdef AGE_DEBUG
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

			age_assertFatal(didFindLayer, "Requested validation layer(%s) is not available.", layer);
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
		createInfo.enabledExtensionCount = static_cast<u32>(extensions.count());
		createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef AGE_DEBUG
		createInfo.enabledLayerCount = static_cast<u32>(k_validationLayers.size());
		createInfo.ppEnabledLayerNames = k_validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
#endif

		AGE_VK_CHECK(vkCreateInstance(&createInfo, nullptr, &_instance));
		age_log(k_tag, "Created Instance.");
	}

	{	// CreateWindowSurface
		AGE_VK_CHECK(glfwCreateWindowSurface(_instance, window, nullptr, &_surface));
		age_log(k_tag, "Created Window Surface.");
	}

#ifdef AGE_DEBUG
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
			age_error(k_tag, "Unable to create DebugUtilsMessenger due to extension not being present.");
		}
	}
#endif

	{	// PickPhysicalDevice
		u32 deviceCount = 0;
		vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
		age_assertFatal(deviceCount > 0, "Unable to find physical devices with Vulkan support.");

		DArray<VkPhysicalDevice> physicalDevices(deviceCount);
		physicalDevices.addEmpty(deviceCount);
		vkEnumeratePhysicalDevices(_instance, &deviceCount, physicalDevices.data());

		_physicalDevice = pickPhysicalDevice(physicalDevices, _surface, queueIndices, _swapchainDetails);
		age_assertFatal(_physicalDevice != VK_NULL_HANDLE, "Unable to find a suitable physical device.");
	}

	{	// CreateLogicalDevice
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
		createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfoArray.count());
		createInfo.pEnabledFeatures = nullptr;	// TODO: Assign
		createInfo.enabledExtensionCount = static_cast<u32>(k_requiredExtensions.size());
		createInfo.ppEnabledExtensionNames = k_requiredExtensions.data();

		// For older versions on Vulkan, it might be necessary to also set the validation layers here.
		// Since this is not necessary at the time of this implementation, let's skip that.
		// TODO: Investigate how this is really currently working as it seems to be generating weird errors.
		createInfo.enabledLayerCount = 0;
		
		AGE_VK_CHECK(vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device));
		age_log(k_tag, "Created logical device.");
	}

	{	// StoreQueueHandles
		for (int i = 0; i < static_cast<u8>(e_QueueFamily::Count); i++)
			vkGetDeviceQueue(_device, queueIndices.indices[i], 0, &_queueArray[i]);
	}


	createSwapchain(window);

	_imageInFlightFences.reserveWithValue(_imageViews.count(), VK_NULL_HANDLE);
}



void VulkanSystem::createSwapchain(GLFWwindow *window)
{

	{	// CreateSwapchain
		const VkSurfaceCapabilitiesKHR &capabilities = _swapchainDetails.capabilities;

		VkColorSpaceKHR colorSpace;
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

		{	// ChooseFormat
			VkSurfaceFormatKHR format = _swapchainDetails.formats[0];

			for (const auto &availableFormat : _swapchainDetails.formats) {
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					format = availableFormat;
					break;
				}
			}
			
			colorSpace = format.colorSpace;
			_swapchainData.format = format.format;
		}

		{	// ChoosePresentMode
			for (const auto &availablePresentMode : _swapchainDetails.presentMode) {
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
					presentMode = availablePresentMode;
			}
		}

		{	// ChooseExtension
			VkExtent2D extent;
			i32 width, height;
			glfwGetFramebufferSize(window, &width, &height);
			extent = { static_cast<u32>(width), static_cast<u32>(height)};

			extent.width = math::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			extent.height = math::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			_swapchainData.extent = extent;
		}

		constexpr u32 k_extraSwapchainImages = 1;

		u32 imageCount = capabilities.minImageCount + k_extraSwapchainImages;
		u32 presentationQueueIndex = _queueIndices.indices[static_cast<u32>(e_QueueFamily::Presentation)];
		u32 graphicsQueueIndex = _queueIndices.indices[static_cast<u32>(e_QueueFamily::Graphics)];
		SArray<u32, 2> swapchainQueueIndices = {presentationQueueIndex, graphicsQueueIndex};

		if (capabilities.maxImageCount > 0 && capabilities.maxImageCount < imageCount) {
			age_warning(k_tag, "Trying to have more swapchain images than what is allowed. SwapChain images count will be clamped to the allowed maximum.");
			imageCount = capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.surface = _surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = _swapchainData.format;
		createInfo.imageColorSpace = colorSpace;
		createInfo.imageExtent = _swapchainData.extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (presentationQueueIndex != graphicsQueueIndex) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = static_cast<u32>(swapchainQueueIndices.size());
			createInfo.pQueueFamilyIndices = swapchainQueueIndices.data();
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		AGE_VK_CHECK(vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapchain));
		age_log(k_tag, "Swapchain created.");
	}

	{	// StoreImageHandles
		u32 imageCount = 0;
		vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);

		_swapchainData.images.reserve(imageCount);
		_swapchainData.images.addEmpty(imageCount);
		vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _swapchainData.images.data());
	}

	{	// CreateImageViews
		const auto &images = _swapchainData.images;
		_imageViews.reserveWithEmpty(images.count());

		for (int i = 0; i < images.count(); i++) {
			VkImageViewCreateInfo createInfo;
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.image = images[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = _swapchainData.format;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			AGE_VK_CHECK(vkCreateImageView(_device, &createInfo, nullptr, &_imageViews[i]))
		}

		age_log(k_tag, "Created %d image views", _imageViews.count());
	}

	{	// createRenderPass
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = _swapchainData.format;
		// TODO: Increase when multisampling gets supported
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		// TODO: Change this to LOAD_OP_DONT_CARE when we have a whole scene being drawn
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		// We want the render pass to start, even if there is no image available.
		// Therefore, we need to set up a subpass dependency in order to go further
		// until the color attachment output stage
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		AGE_VK_CHECK(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass));
	}

	{	// createFramebuffers
		_framebuffers.reserveWithEmpty(_imageViews.count());

		for (int i = 0; i < _imageViews.count(); i++) {
			VkImageView attachments[] = {_imageViews[i]};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = _renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = _swapchainData.extent.width;
			framebufferInfo.height = _swapchainData.extent.height;
			framebufferInfo.layers = 1;
			
			AGE_VK_CHECK(vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &(_framebuffers[i])));
		}

		age_log(k_tag, "Created %d framebuffers", _framebuffers.count());
	}

	{	// createCommandPool
		VkCommandPoolCreateInfo commandPoolInfo = {};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex = _queueIndices.indices[static_cast<i32>(e_QueueFamily::Graphics)];

		AGE_VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_graphicsCommandPool));

		age_log(k_tag, "Created Graphics Command Pool");
	}

	{	// createFrameData
		for (FrameSyncData &data : _frameSyncData)
			createFrameData(data);

		age_log(k_tag, "Created Frame Data");
	}
}



void VulkanSystem::createFrameData(FrameSyncData &frameData)
{
	{	// createSemaphores
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		AGE_VK_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &frameData.imageAvailableSemaphore));
		AGE_VK_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &frameData.renderFinishedSemaphore));
	}

	{ // createFence
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		AGE_VK_CHECK(vkCreateFence(_device, &fenceInfo, nullptr, &frameData.inFlightFence));
	}
}


void VulkanSystem::destroyFrameData(FrameSyncData &frameData)
{
	vkDestroySemaphore(_device, frameData.renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(_device, frameData.imageAvailableSemaphore, nullptr);

	vkDestroyFence(_device, frameData.inFlightFence, nullptr);
}

void VulkanSystem::cleanup()
{
	for (FrameSyncData &data : _frameSyncData)
		destroyFrameData(data);

	vkDestroyCommandPool(_device, _graphicsCommandPool, nullptr);

	for (VkFramebuffer framebuffer : _framebuffers) {
		vkDestroyFramebuffer(_device, framebuffer, nullptr);
	}

	vkDestroyRenderPass(_device, _renderPass, nullptr);

	for (VkImageView imageView : _imageViews) {
		vkDestroyImageView(_device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(_device, _swapchain, nullptr);
	vkDestroyDevice(_device, nullptr);

#ifdef AGE_DEBUG
	{	// DestroyDebugUtilsMessenger
		auto f_destroyDebugUtilsMessengerEXT = AGE_VK_GET_COMMAND(_instance, vkDestroyDebugUtilsMessengerEXT);
		if (f_destroyDebugUtilsMessengerEXT != nullptr)
			f_destroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
	}
#endif

	vkDestroySurfaceKHR(_instance, _surface, nullptr);
	vkDestroyInstance(_instance, nullptr);
}

void VulkanSystem::draw(const CommandBufferArray &commandBuffers)
{
	age_assertFatal(commandBuffers.count() == _swapchainData.images.count(), "There must be as many command buffers as swap chain images.");
	age_assertFatal(_currentFrame < k_maxFramesInFlight ,"");

	FrameSyncData &currentFrameData = _frameSyncData[_currentFrame];
	vkWaitForFences(_device, 1, &currentFrameData.inFlightFence, VK_TRUE, UINT64_MAX);

	u32 imageIndex;
	vkAcquireNextImageKHR(_device, _swapchain, UINT64_MAX, currentFrameData.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	{	// SyncImageInFlight

		// Wait if the acquired image is in flight
		if (_imageInFlightFences[imageIndex] != VK_NULL_HANDLE)
			vkWaitForFences(_device, 1, &_imageInFlightFences[imageIndex], VK_TRUE, UINT64_MAX);

		// Store current fence for the indexed image
		_imageInFlightFences[imageIndex] = currentFrameData.inFlightFence;
	}

	{	// Submit
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

		const VkQueue graphicsQueue = _queueArray[static_cast<i32>(e_QueueFamily::Graphics)];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &currentFrameData.imageAvailableSemaphore;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &currentFrameData.renderFinishedSemaphore;

		vkResetFences(_device, 1, &currentFrameData.inFlightFence);
		AGE_VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, currentFrameData.inFlightFence));
	}

	{	// Present
		const VkQueue presentQueue = _queueArray[static_cast<i32>(e_QueueFamily::Presentation)];

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &currentFrameData.renderFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &_swapchain;
		presentInfo.pImageIndices = &imageIndex;

		AGE_VK_CHECK(vkQueuePresentKHR(presentQueue, &presentInfo));

		vkQueueWaitIdle(presentQueue);
	}

	_currentFrame = (_currentFrame + 1) % k_maxFramesInFlight;
}

CommandBufferArray VulkanSystem::allocDrawCommandBuffer(PipelineHandle pipelineHandle) const
{
	// TODO: Track command buffer allocation
	age_log(k_tag, "Draw Command Buffer Allocated");

	// TODO: Make the clear color configurable
	VkClearValue clearColor = {{{0.2f, 0.2f, 0.2f, 1.0f}}};
	VkPipeline pipeline = PipelineSystem::s_inst.get(pipelineHandle).pipeline;

	const u32 bufferCount = static_cast<u32>(_framebuffers.count());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _graphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = bufferCount;

	CommandBufferArray commandBuffers(bufferCount);
	commandBuffers.addEmpty(bufferCount);

	AGE_VK_CHECK(vkAllocateCommandBuffers(_device, &allocInfo, commandBuffers.data()));

	for (int i = 0; i < commandBuffers.count(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		AGE_VK_CHECK(vkBeginCommandBuffer(commandBuffers[i], &beginInfo));

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = _renderPass;
		renderPassInfo.framebuffer = _framebuffers[i];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = _swapchainData.extent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		// TODO: Adapt this to proper meshes
		vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(commandBuffers[i]);

		AGE_VK_CHECK(vkEndCommandBuffer(commandBuffers[i]));
	}
	
	return commandBuffers;
}

void VulkanSystem::freeDrawCommandBuffers(CommandBufferArray &commandBuffers) const
{
	vkFreeCommandBuffers(_device, _graphicsCommandPool, static_cast<u32>(_framebuffers.count()), commandBuffers.data());
	commandBuffers.clear();
}

void VulkanSystem::waitForAllFrames() const
{
	age_log(k_tag, "Waiting for all frames to finish...");

	for (const FrameSyncData &data : _frameSyncData)
		vkWaitForFences(_device, 1, &data.inFlightFence, VK_TRUE, UINT64_MAX);
}

}
