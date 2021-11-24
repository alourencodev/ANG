#include <AGE/Renderer/Vulkan/Vulkan.h>

#include <vulkan/vulkan.h>

#include <Core/BuildScheme.hpp>
#include <Core/BitField.hpp>
#include <Core/DArray.hpp>
#include <Core/File.h>
#include <Core/HashMap.hpp>
#include <Core/Log/Log.h>

#include <AGE/Vendor/GLFW.hpp>
#include <AGE/Renderer/Vulkan/VulkanUtils.h>

namespace age::vk
{

constexpr char k_tag[] = "VulkanBootstrap";
constexpr u8 k_maxFramesInFlight = 2;



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



// If a new family is added to the enum, it is also necessary to check if the physical device supports it.
enum class e_QueueFamily : u8
{
	Graphics,
	Presentation,

	Count
};

using QueueIndexArray = SArray<u32, static_cast<u8>(e_QueueFamily::Count)>;
using QueueArray = SArray<VkQueue, static_cast<u8>(e_QueueFamily::Count)>;



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



struct Shader
{
	VkShaderModule module;
	VkShaderStageFlagBits stage;
};



struct Pipeline
{
	VkPipelineLayout layout = {};
	VkViewport viewport = {};
	VkRect2D scissor = {};
	VkPipeline pipeline = VK_NULL_HANDLE;
	PipelineCreateInfo createInfo;
};



struct DrawCommand
{
	DArray<VkCommandBuffer> buffers;
	PipelineHandle pipeline;
	MeshBufferHandle meshBuffer;
};


struct MeshBuffer
{
	Buffer buffer;
	u32 vertexCount;
};


/**
@brief	The Context holds every data necessary to interface with Vulkan. 
		This is data that doesn't change during an entire sesion.
**/
struct Context
{
	QueueIndexArray queueIndices = {};
	QueueArray queues = {};
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
		for rendering. It must be recreated if the widow changes (e.g. resize).
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
	u8 currentFrame = 0;
};



/**
@brief	Holds every resource that is referenced externally through handles.
**/
struct Resources
{
	DArray<Shader> shaders;
	DArray<Pipeline> pipelines;
	HashMap<u32, DrawCommand> drawCommandBuffers;
	HashMap<u32, MeshBuffer> meshBuffers;
};



// Static Vulkan State
static GLFWwindow *s_window = nullptr;
static Context s_context;
static RenderEnvironment s_environment;
static Resources s_resources;



// Handle counters
static u32 s_drawCommandHandleCounter = 0;
static u32 s_meshBufferHandleCounter = 0;



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


	{	// Create Command Pool
		VkCommandPoolCreateInfo commandPoolInfo = {};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex = s_context.queueIndices[static_cast<i32>(e_QueueFamily::Graphics)];

		AGE_VK_CHECK(vkCreateCommandPool(s_context.device, &commandPoolInfo, nullptr, &s_context.graphicsCommandPool));

		age_log(k_tag, "Created Graphics Command Pool");
	}


	{	// Store Queue Handles
		for (int i = 0; i < static_cast<u8>(e_QueueFamily::Count); i++)
			vkGetDeviceQueue(s_context.device, s_context.queueIndices[i], 0, &s_context.queues[i]);
	}
}



void cleanupContext()
{
	age_log(k_tag, "Cleaning up Context");

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
		s_environment.frameSyncData.resizeWithEmpty(k_maxFramesInFlight);
		for (FrameSyncData &data : s_environment.frameSyncData)
			createFrameData(data);

		age_log(k_tag, "Created Frame Data");
	}

	s_environment.imageInFlightFences.resizeWithValue(s_environment.images.count(), VK_NULL_HANDLE);
}



void cleanupRenderEnvironment()
{
	age_log(k_tag, "Cleaning up Render Environment");

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

	vkDestroyRenderPass(s_context.device, s_environment.renderPass, nullptr);
}



void cleanupPipeline(const Pipeline &pipeline)
{
	vkDestroyPipeline(s_context.device, pipeline.pipeline, nullptr);
	vkDestroyPipelineLayout(s_context.device, pipeline.layout, nullptr);
}



void cleanupResources()
{
	// Check if resources that are owned externally were cleaned.
	age_assertFatal(s_resources.drawCommandBuffers.isEmpty(), "Resources are being cleaned, but %d drawCommands still exist. Every drawCommand should be cleaned.", s_resources.drawCommandBuffers.count());
	age_assertFatal(s_resources.meshBuffers.isEmpty(), "Resources are being cleaned, but %d meshBuffers still exist. Every meshBuffer should be cleaned.", s_resources.meshBuffers.count());

	{	// Cleanup Shaders
		age_log(k_tag, "Cleaning up shaders.");

		for (const Shader &shader : s_resources.shaders)
			vkDestroyShaderModule(s_context.device, shader.module, nullptr);

		s_resources.shaders.clear();
	}
	

	{	// Cleanup Pipelines
		age_log(k_tag, "Cleaning up pipelines.");

		for (const Pipeline &pipeline : s_resources.pipelines)
			cleanupPipeline(pipeline);

		s_resources.pipelines.clear();
	}
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
	cleanupResources();
	cleanupContext();
}



ShaderHandle createShader(e_ShaderStage shaderStage, const char *path)
{
	age_log(k_tag, "Creating shader from %s", path);

	// This source will be discarded at the end of the function, since it's not needed anymore.
	DArray<char> source = file::readBinary(path);

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.codeSize = static_cast<u32>(source.count());
	createInfo.pCode = reinterpret_cast<const u32 *>(source.data());

	Shader shader{};
	AGE_VK_CHECK(vkCreateShaderModule(s_context.device, &createInfo, nullptr, &shader.module));

	switch (shaderStage)
	{
		case e_ShaderStage::Vertex: {
			shader.stage = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		}
		case e_ShaderStage::Fragment: {
			shader.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		}
		default: {
			age_assertFatal(false, "Trying to create a shader with an unhandled shader stage");
		}
	}

	ShaderHandle handle(static_cast<u32>(s_resources.shaders.count()));
	s_resources.shaders.add(shader);

	return handle;
}



VkPipelineShaderStageCreateInfo createShaderStage(const Shader &shader)
{
	VkPipelineShaderStageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.module = shader.module;
	info.stage = shader.stage;
	info.pName = "main";

	return info;
}



Pipeline createPipelineInternal(const PipelineCreateInfo &info)
{
	// TODO: Be more clear about what Pipeline are we making as soon as it gets a name
	age_log(k_tag, "Creating Vulkan Pipeline.");

	Pipeline pipeline = {};
	pipeline.createInfo = info;

	// Create Shader Stages
	StackArray<VkPipelineShaderStageCreateInfo, static_cast<u32>(e_ShaderStage::Count)> shaderStages;
	for (ShaderHandle shaderHandle : info.shaders) {
		age_assertFatal(shaderHandle.isValid(), "Trying to create pipeline with invalid shader.");
		age_assertFatal(shaderHandle < s_resources.shaders.count(), "Trying to create pipeline with inexistent shader.");

		shaderStages.add(createShaderStage(s_resources.shaders[shaderHandle]));
	}


	{	// Create Pipeline Layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		// TODO: Properly create the layout

		AGE_VK_CHECK(vkCreatePipelineLayout(s_context.device, &pipelineLayoutInfo, nullptr, &pipeline.layout));
	}


	{	// Create Pipeline

		// Set vertex Attributes
		constexpr u8 k_attributesPerVertex = 1;
		SArray<VkVertexInputAttributeDescription, k_attributesPerVertex> attributesDescription;
		{
			// pos
			attributesDescription[0].binding = 0;
			attributesDescription[0].location = 0;
			attributesDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributesDescription[0].offset = offsetof(Vertex, pos);
		}

		VkVertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkPipelineVertexInputStateCreateInfo vertexInput = {};
		vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInput.vertexBindingDescriptionCount = 1;
		vertexInput.pVertexBindingDescriptions = &bindingDescription;
		vertexInput.vertexAttributeDescriptionCount = k_attributesPerVertex;
		vertexInput.pVertexAttributeDescriptions = attributesDescription.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		const VkExtent2D &swapChainExtent = s_environment.swapchainExtent;
		VkViewport &viewport = pipeline.viewport;
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<f32>(swapChainExtent.width);
		viewport.height = static_cast<f32>(swapChainExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		pipeline.scissor.offset = {0, 0};
		pipeline.scissor.extent = swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &pipeline.viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &pipeline.scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		// TODO: Enable multisampling
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		// TODO: Handle color blending
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = static_cast<u32>(shaderStages.count());
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInput;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.layout = pipeline.layout;
		pipelineInfo.renderPass = s_environment.renderPass;
		pipelineInfo.subpass = 0;
		
		AGE_VK_CHECK(vkCreateGraphicsPipelines(s_context.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.pipeline));
	}

	return pipeline;
}



PipelineHandle createPipeline(const PipelineCreateInfo &info)
{
	PipelineHandle handle(static_cast<u32>(s_resources.pipelines.count()));
	s_resources.pipelines.add(createPipelineInternal(info));

	return handle;
}



DrawCommand createDrawCommandInternal(const PipelineHandle &pipelineHandle, const MeshBufferHandle &meshBufferHandle)
{
	age_assertFatal(pipelineHandle.isValid(), "Trying to create a draw command with an invalid pipeline handle.");

	// TODO: Track command buffer allocation
	age_log(k_tag, "Draw Command Buffer Allocated");

	// TODO: Make the clear color configurable
	VkClearValue clearColor = {{{0.2f, 0.2f, 0.2f, 1.0f}}};
	VkPipeline pipeline = s_resources.pipelines[pipelineHandle].pipeline;

	const u32 bufferCount = static_cast<u32>(s_environment.framebuffers.count());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = s_context.graphicsCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = bufferCount;

	DrawCommand command;
	command.buffers.reserveWithEmpty(bufferCount);
	command.pipeline = pipelineHandle;
	command.meshBuffer = meshBufferHandle;

	AGE_VK_CHECK(vkAllocateCommandBuffers(s_context.device, &allocInfo, command.buffers.data()));

	const MeshBuffer meshBuffer = s_resources.meshBuffers[meshBufferHandle];

	for (int i = 0; i < command.buffers.count(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		AGE_VK_CHECK(vkBeginCommandBuffer(command.buffers[i], &beginInfo));

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = s_environment.renderPass;
		renderPassInfo.framebuffer = s_environment.framebuffers[i];
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = s_environment.swapchainExtent;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		{	// Record Command Buffer
			vkCmdBeginRenderPass(command.buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(command.buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			const VkDeviceSize offsets[] = {0};
			vkCmdBindVertexBuffers(command.buffers[i], 0, 1, &meshBuffer.buffer.buffer, offsets);

			vkCmdDraw(command.buffers[i], meshBuffer.vertexCount, 1, 0, 0);

			vkCmdEndRenderPass(command.buffers[i]);
		}

		AGE_VK_CHECK(vkEndCommandBuffer(command.buffers[i]));
	}

	return command;
}



DrawCommandHandle createDrawCommand(const PipelineHandle &pipelineHandle, const MeshBufferHandle &meshBufferHandle)
{
	DrawCommandHandle handle(s_drawCommandHandleCounter);
	s_drawCommandHandleCounter++;
	s_resources.drawCommandBuffers.add(handle, createDrawCommandInternal(pipelineHandle, meshBufferHandle));
	
	return handle;
}



void cleanupDrawCommandInternal(DrawCommand &drawCommand)
{
	DArray<VkCommandBuffer> &buffers = drawCommand.buffers;
	vkFreeCommandBuffers(s_context.device, s_context.graphicsCommandPool, 
						 static_cast<u32>(buffers.count()), buffers.data());
	buffers.clear();
}



void cleanupDrawCommand(DrawCommandHandle &commandHandle)
{
	age_assertFatal(commandHandle.isValid(), "Trying to cleanup an invalid draw command.");

	cleanupDrawCommandInternal(s_resources.drawCommandBuffers[commandHandle]);
	s_resources.drawCommandBuffers.remove(commandHandle);
	commandHandle = DrawCommandHandle::invalid();
}



MeshBufferHandle createMeshBuffer(const DArray<Vertex> &vertices)
{
	age_assertFatal(!vertices.isEmpty(), "Unable to create MeshBuffer with no vertices.");

	MeshBufferHandle handle(s_meshBufferHandleCounter);
	s_meshBufferHandleCounter++;

	const size_t bufferSize = sizeof(vertices[0]) * vertices.count();
	MeshBuffer meshBuffer;
	meshBuffer.vertexCount = static_cast<u32>(vertices.count());
	meshBuffer.buffer = vk::allocBuffer(s_context.physicalDevice, s_context.device, bufferSize);

	{	// Fill Buffer
		void *data;
		vkMapMemory(s_context.device, meshBuffer.buffer.memory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), bufferSize);
		vkUnmapMemory(s_context.device, meshBuffer.buffer.memory);
	}

	s_resources.meshBuffers.add(handle, meshBuffer);

	return handle;
}



void cleanupMeshBuffer(MeshBufferHandle &meshBufferHandle)
{
	age_assertFatal(meshBufferHandle.isValid(), "Trying to cleanup an invalid mesh buffer.");

	MeshBuffer &meshBuffer = s_resources.meshBuffers[meshBufferHandle];
	s_resources.meshBuffers.remove(meshBufferHandle);

	vk::freeBuffer(s_context.device, meshBuffer.buffer);

	meshBufferHandle = MeshBufferHandle::invalid();
}



void recreateRenderEnvironment()
{
	age_log(k_tag, "Recreating Render Environment...");

	vkDeviceWaitIdle(s_context.device);

	cleanupRenderEnvironment();
	createRenderEnvironment();

	// Recreate Pipelines
	for (Pipeline &pipeline : s_resources.pipelines) {
		cleanupPipeline(pipeline);
		pipeline = createPipelineInternal(pipeline.createInfo);
	}

	// Recreate DrawCommands
	for (auto &drawCommandEntry : s_resources.drawCommandBuffers.asRange()) {
		if (!drawCommandEntry.isValid())
			continue;

		DrawCommand &drawCommand = drawCommandEntry.value;

		cleanupDrawCommandInternal(drawCommand);
		drawCommand = createDrawCommandInternal(drawCommand.pipeline, drawCommand.meshBuffer);
	}

	age_log(k_tag, "Render Environment recreated.");
}



void draw(const DrawCommandHandle &commandBufferHandle)
{
	const DrawCommand &commandBuffer = s_resources.drawCommandBuffers[commandBufferHandle];
	age_assertFatal(commandBuffer.buffers.count() == s_environment.images.count(), "There must be as many command buffers as swap chain images.");
	age_assertFatal(s_environment.currentFrame < k_maxFramesInFlight, "");

	FrameSyncData currentSyncData = s_environment.frameSyncData[s_environment.currentFrame];
	vkWaitForFences(s_context.device, 1, &currentSyncData.inFlightFence, VK_TRUE, UINT64_MAX);

	u32 imageIndex;

	{	// Acquire Image
		VkResult result = vkAcquireNextImageKHR(s_context.device, s_environment.swapchain, UINT64_MAX, 
												currentSyncData.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR)
			recreateRenderEnvironment();
		else
			AGE_VK_CHECK(result);
	}


	{	// SyncImageInFlight

		// Wait if the acquired image is in flight
		VkFence &imageInFlightFence = s_environment.imageInFlightFences[imageIndex];
		if (imageInFlightFence != VK_NULL_HANDLE)
			vkWaitForFences(s_context.device, 1, &imageInFlightFence, VK_TRUE, UINT64_MAX);

		// Store current fence for the indexed image
		imageInFlightFence = currentSyncData.inFlightFence;
	}


	{	// Submit
		VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

		const VkQueue graphicsQueue = s_context.queues[static_cast<i32>(e_QueueFamily::Graphics)];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &currentSyncData.imageAvailableSemaphore;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer.buffers[imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &currentSyncData.renderFinishedSemaphore;

		vkResetFences(s_context.device, 1, &currentSyncData.inFlightFence);
		AGE_VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, currentSyncData.inFlightFence));
	}


	{	// Present
		const VkQueue presentQueue = s_context.queues[static_cast<i32>(e_QueueFamily::Presentation)];

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &currentSyncData.renderFinishedSemaphore;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &s_environment.swapchain;
		presentInfo.pImageIndices = &imageIndex;

		VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
			recreateRenderEnvironment();
		else
			AGE_VK_CHECK(result);

		vkQueueWaitIdle(presentQueue);
	}

	s_environment.currentFrame = (s_environment.currentFrame + 1) % k_maxFramesInFlight;
}



void waitForFramesToFinish()
{
	for (const auto &frameSyncData : s_environment.frameSyncData)
		vkWaitForFences(s_context.device, 1, &frameSyncData.inFlightFence, VK_TRUE, INT64_MAX);
}

}	// namespace age::vk