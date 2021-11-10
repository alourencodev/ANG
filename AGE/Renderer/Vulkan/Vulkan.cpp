#include <AGE/Renderer/Vulkan/Vulkan.h>

#include <vulkan/vulkan.h>

#include <Core/BuildScheme.hpp>
#include <Core/BitField.hpp>
#include <Core/DArray.hpp>
#include <Core/Log/Log.h>

#include <AGE/Vendor/GLFW.hpp>
#include <AGE/Renderer/Vulkan/VulkanUtils.h>

namespace age::vk
{

constexpr char k_tag[] = "VulkanBootstrap";



static const SArray<const char *, 1> k_requiredExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef AGE_DEBUG

static const SArray<const char *, 1> k_validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

static const SArray<const char *, 1> k_debugExtensions = {
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

#endif	// AGE_DEBUG



// If a new family is added to the enum, it is also necessary to check if the physical device supports it.
enum class e_QueueFamily : u8
{
	Graphics,
	Presentation,

	Count
};

using QueueIndexArray = SArray<u32, static_cast<u8>(e_QueueFamily::Count)>;



/**
@brief	Helper structure that allows to quick check queue existence by queue family.
**/
struct QueueIndexBitMap
{
	QueueIndexArray indices = {};
	BitField map;
};



struct FrameSyncData
{
	VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
	VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
	VkFence inFlightFence = VK_NULL_HANDLE;
};



struct SurfaceData
{
	VkSurfaceCapabilitiesKHR capabilities;
	DArray<VkSurfaceFormatKHR> formats;
	DArray<VkPresentModeKHR> presentMode;
};



/**
@brief	The Context holds every data necessary to interface with Vulkan. 
		This is data that doesn't change during an entire sesion.
**/
struct Context
{
	QueueIndexArray queueIndices = {};
	VkInstance instance = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;
	VkCommandPool graphicsCommandPool = VK_NULL_HANDLE;

#ifdef AGE_DEBUG
	VkDebugUtilsMessengerEXT debugMessenger;
#endif // AGE_DEBUG
};



/**
@brief	The RendenderEnvironment holds the vulkan structures that are directly necessary
		for rendering. It must be created if the widow changes (e.g. resize).
**/
struct RenderEnvironment
{
	DArray<VkImage> images = {};
	DArray<VkImageView> imageViews = {};
	DArray<VkFramebuffer> framebuffers = {};
	DArray<FrameSyncData> frameSyncData = {};
	DArray<VkFence> imageInFlightFences = {};
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	VkExtent2D swapchainExtent = {0, 0};
	VkRenderPass renderPass = VK_NULL_HANDLE;
};



// Static Vulkan State
static GLFWwindow *s_window = nullptr;
static Context s_context;
static RenderEnvironment s_environment;



DArray<const char *> getRequiredExtensions()
{
	DArray<const char *> extensions;

	u32 glfwExtensionCount = 0;
	const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	extensions.reserve(glfwExtensionCount IF_DEBUG(+ k_debugExtensions.size()));

#ifdef AGE_DEBUG
	extensions.add(k_debugExtensions);
#endif
	extensions.add(glfwExtensions, glfwExtensionCount);

	return extensions;
}



#ifdef AGE_DEBUG
void checkValidationLayersAvailability()
{
	u32 layerCount;
	AGE_VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

	DArray<VkLayerProperties> availableLayerProperties(layerCount);
	availableLayerProperties.addEmpty(layerCount);
	AGE_VK_CHECK(vkEnumerateInstanceLayerProperties(&layerCount, availableLayerProperties.data()));

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
				const u8 graphicsIndex = static_cast<u8>(e_QueueFamily::Graphics);
				queueIndices.map.set(graphicsIndex);
				queueIndices.indices[graphicsIndex] = i;
			}
		}

		// Presentation Queue
		{
			VkBool32 supportsPresentation = false;
			AGE_VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresentation));

			if (supportsPresentation) {
				const u8 presentationIndex = static_cast<u8>(e_QueueFamily::Presentation);
				queueIndices.map.set(presentationIndex);
				queueIndices.indices[presentationIndex] = i;
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



bool isDeviceCompatible(VkPhysicalDevice physicalDevice, const QueueIndexBitMap &queueIndexBitMap, const SurfaceData &swapChainDetails)
{
	// CheckRequiredQueueFamilies
	if (!queueIndexBitMap.map.isSetBelow(static_cast<u8>(e_QueueFamily::Count)))
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



VkPhysicalDevice pickPhysicalDevice(const DArray<VkPhysicalDevice> &candidates, VkSurfaceKHR surface, QueueIndexArray &o_queueIndices)
{
	// TODO https://trello.com/c/FZ8pfoMI
	// Currently we just pick the first dedicated GPU. 
	// If there is no dedicated GPU we fallback to the first candidate.
	// This is a very naive way to pick a GPU, so a better way should be implemented in the future.

	for (VkPhysicalDevice candidate : candidates) {
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(candidate, &properties);
		
		QueueIndexBitMap queueIndexBitMap = getDeviceQueueIndices(candidate, surface);
		SurfaceData swapChainDetails = getSurfaceData(candidate, surface);
		if (!isDeviceCompatible(candidate, queueIndexBitMap, swapChainDetails))
			continue;

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			age_log(k_tag, "Picked %s as a physical device", properties.deviceName);
			o_queueIndices = queueIndexBitMap.indices;

			return candidate;
		}
	}

	return VK_NULL_HANDLE;
}



void createFrameData(FrameSyncData &frameData)
{
	{	// createSemaphores
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		AGE_VK_CHECK(vkCreateSemaphore(s_context.device, &semaphoreInfo, nullptr, &frameData.imageAvailableSemaphore));
		AGE_VK_CHECK(vkCreateSemaphore(s_context.device, &semaphoreInfo, nullptr, &frameData.renderFinishedSemaphore));
	}

	{ // createFence
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		AGE_VK_CHECK(vkCreateFence(s_context.device, &fenceInfo, nullptr, &frameData.inFlightFence));
	}
}



void destroyFrameData(FrameSyncData &frameData)
{
	vkDestroySemaphore(s_context.device, frameData.renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(s_context.device, frameData.imageAvailableSemaphore, nullptr);

	vkDestroyFence(s_context.device, frameData.inFlightFence, nullptr);
}



void createContext()
{
	const auto extensions = getRequiredExtensions();

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
		createInfo.enabledExtensionCount = static_cast<u32>(extensions.count());
		createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef AGE_DEBUG
		createInfo.enabledLayerCount = static_cast<u32>(k_validationLayers.size());
		createInfo.ppEnabledLayerNames = k_validationLayers.data();
#else
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
#endif

		AGE_VK_CHECK(vkCreateInstance(&createInfo, nullptr, &s_context.instance));
		age_log(k_tag, "Created Instance.");
	}


	#ifdef AGE_DEBUG
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

		auto f_createDebugUtilsMessengerEXT = AGE_VK_GET_COMMAND(s_context.instance, vkCreateDebugUtilsMessengerEXT);
		if (f_createDebugUtilsMessengerEXT != nullptr) {
			AGE_VK_CHECK(f_createDebugUtilsMessengerEXT(s_context.instance, &createInfo, nullptr, &s_context.debugMessenger));
		} else {
			age_error(k_tag, "Unable to create DebugUtilsMessenger due to extension not being present.");
		}
	}
#endif


	{	// Create Surface
		AGE_VK_CHECK(glfwCreateWindowSurface(s_context.instance, s_window, nullptr, &s_context.surface));
		age_log(k_tag, "Created Window Surface.");
	}



	{	// Pick Physical Device
		u32 deviceCount = 0;
		vkEnumeratePhysicalDevices(s_context.instance, &deviceCount, nullptr);
		age_assertFatal(deviceCount > 0, "Unable to find physical devices with Vulkan support.");

		DArray<VkPhysicalDevice> physicalDevices(deviceCount);
		physicalDevices.addEmpty(deviceCount);
		vkEnumeratePhysicalDevices(s_context.instance, &deviceCount, physicalDevices.data());

		s_context.physicalDevice = pickPhysicalDevice(physicalDevices, s_context.surface, s_context.queueIndices);
		age_assertFatal(s_context.instance != VK_NULL_HANDLE, "Unable to find a suitable physical device.");
	}


	{	// Create Logical Device
		DArray<VkDeviceQueueCreateInfo> queueCreateInfoArray(static_cast<u32>(e_QueueFamily::Count));

		float queuePriority = 1.0f;
		for (int i = 0; i < static_cast<u32>(e_QueueFamily::Count); i++) {
			VkDeviceQueueCreateInfo queueCreateInfo;
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.pNext = nullptr;
			queueCreateInfo.flags = 0;
			queueCreateInfo.queueFamilyIndex = s_context.queueIndices[i];
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
		createInfo.enabledLayerCount = 0;
		
		AGE_VK_CHECK(vkCreateDevice(s_context.physicalDevice, &createInfo, nullptr, &s_context.device));
		age_log(k_tag, "Created logical device.");
	}


	{	// createCommandPool
		VkCommandPoolCreateInfo commandPoolInfo = {};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex = s_context.queueIndices[static_cast<i32>(e_QueueFamily::Graphics)];

		AGE_VK_CHECK(vkCreateCommandPool(s_context.device, &commandPoolInfo, nullptr, &s_context.graphicsCommandPool));

		age_log(k_tag, "Created Graphics Command Pool");
	}
}



void cleanupContext()
{
	vkDestroyCommandPool(s_context.device, s_context.graphicsCommandPool, nullptr);
	vkDestroyDevice(s_context.device, nullptr);

#ifdef AGE_DEBUG
	{	// DestroyDebugUtilsMessenger
		auto destroyDebugUtilsMessengerEXT = AGE_VK_GET_COMMAND(s_context.instance, vkDestroyDebugUtilsMessengerEXT);
		if (destroyDebugUtilsMessengerEXT != nullptr)
			destroyDebugUtilsMessengerEXT(s_context.instance, s_context.debugMessenger, nullptr);
	}
#endif

	vkDestroySurfaceKHR(s_context.instance, s_context.surface, nullptr);
	vkDestroyInstance(s_context.instance, nullptr);
}



void createRenderEnvironment()
{
	VkFormat swapchainFormat = VK_FORMAT_UNDEFINED;

	{	// Create Swapchain
		SurfaceData surfaceData = getSurfaceData(s_context.physicalDevice, s_context.surface);
		const VkSurfaceCapabilitiesKHR &capabilities = surfaceData.capabilities;

		VkColorSpaceKHR colorSpace;
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

		{	// Choose Format
			VkSurfaceFormatKHR format = surfaceData.formats[0];

			for (const auto &availableFormat : surfaceData.formats) {
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
					format = availableFormat;
					break;
				}
			}
			
			colorSpace = format.colorSpace;
			swapchainFormat = format.format;
		}

		{	// Choose PresentMode
			for (const auto &availablePresentMode : surfaceData.presentMode) {
				if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
					presentMode = availablePresentMode;
			}
		}

		{	// Choose Extension
			VkExtent2D extent;
			i32 width, height;
			glfwGetFramebufferSize(s_window, &width, &height);
			extent = { static_cast<u32>(width), static_cast<u32>(height)};

			extent.width = math::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			extent.height = math::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			s_environment.swapchainExtent = extent;
		}

		constexpr u32 k_extraSwapchainImages = 1;

		u32 imageCount = capabilities.minImageCount + k_extraSwapchainImages;
		u32 presentationQueueIndex = s_context.queueIndices[static_cast<u32>(e_QueueFamily::Presentation)];
		u32 graphicsQueueIndex = s_context.queueIndices[static_cast<u32>(e_QueueFamily::Graphics)];
		SArray<u32, 2> swapchainQueueIndices = {presentationQueueIndex, graphicsQueueIndex};

		if (capabilities.maxImageCount > 0 && capabilities.maxImageCount < imageCount) {
			age_warning(k_tag, "Trying to have more swapchain images than what is allowed. SwapChain images count will be clamped to the allowed maximum.");
			imageCount = capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.surface = s_context.surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = swapchainFormat;
		createInfo.imageColorSpace = colorSpace;
		createInfo.imageExtent = s_environment.swapchainExtent;
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

		AGE_VK_CHECK(vkCreateSwapchainKHR(s_context.device, &createInfo, nullptr, &s_environment.swapchain));
		age_log(k_tag, "Swapchain created.");
	}


	{	// Store Image Handles
		u32 imageCount = 0;
		vkGetSwapchainImagesKHR(s_context.device, s_environment.swapchain, &imageCount, nullptr);

		s_environment.images.reserveWithEmpty(imageCount);
		vkGetSwapchainImagesKHR(s_context.device, s_environment.swapchain, &imageCount, s_environment.images.data());
	}


	{	// Create Image Views
		const auto &images = s_environment.images;
		s_environment.imageViews.reserveWithEmpty(images.count());

		for (int i = 0; i < images.count(); i++) {
			VkImageViewCreateInfo createInfo;
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.image = images[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapchainFormat;

			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			AGE_VK_CHECK(vkCreateImageView(s_context.device, &createInfo, nullptr, &s_environment.imageViews[i]))
		}

		age_log(k_tag, "Created %d image views", s_environment.imageViews.count());
	}

	{	// Create Render Pass
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swapchainFormat;
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

		AGE_VK_CHECK(vkCreateRenderPass(s_context.device, &renderPassInfo, nullptr, &s_environment.renderPass));
	}

	{	// Create Framebuffers
		const auto &imageViews = s_environment.imageViews;
		s_environment.framebuffers.reserveWithEmpty(imageViews.count());

		for (int i = 0; i < imageViews.count(); i++) {
			VkImageView attachments[] = {imageViews[i]};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = s_environment.renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = s_environment.swapchainExtent.width;
			framebufferInfo.height = s_environment.swapchainExtent.height;
			framebufferInfo.layers = 1;
			
			AGE_VK_CHECK(vkCreateFramebuffer(s_context.device, &framebufferInfo, nullptr, &(s_environment.framebuffers[i])));
		}

		age_log(k_tag, "Created %d framebuffers", s_environment.framebuffers.count());
	}

	{	// Create Frame Sync Data
		for (FrameSyncData &data : s_environment.frameSyncData)
			createFrameData(data);

		age_log(k_tag, "Created Frame Data");
	}

	s_environment.imageInFlightFences.resizeWithValue(s_environment.frameSyncData.count(), VK_NULL_HANDLE);
}



void cleanupRenderEnvironment()
{
	s_environment.imageInFlightFences.clear();

	for (FrameSyncData &data : s_environment.frameSyncData)
		destroyFrameData(data);

	for (VkFramebuffer framebuffer : s_environment.framebuffers)
		vkDestroyFramebuffer(s_context.device, framebuffer, nullptr);
	s_environment.framebuffers.clear();

	for (VkImageView imageView : s_environment.imageViews)
		vkDestroyImageView(s_context.device, imageView, nullptr);
	s_environment.imageViews.clear();

	// The actual images get destroyed when the swapchain is destroyed.
	s_environment.images.clear();
	vkDestroySwapchainKHR(s_context.device, s_environment.swapchain, nullptr);
}



void init(GLFWwindow *window)
{
	s_window = window;
	createContext();
	createRenderEnvironment();
}



void cleanup()
{
	cleanupRenderEnvironment();
	cleanupContext();
}

}