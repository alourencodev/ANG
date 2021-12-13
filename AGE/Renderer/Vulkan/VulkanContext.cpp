
#include <AGE/Renderer/Vulkan/VulkanContext.h>
#include <AGE/Renderer/Vulkan/VulkanPhysicalDevice.h>
#include <AGE/Renderer/Vulkan/VulkanUtils.h>
#include <AGE/Vendor/GLFW.hpp>

#include <Core/DArray.hpp>


namespace age::vk
{

constexpr const char k_tag[] = "VulkanBootstrap";

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



void createInstance(Context &context, const ContextCreateInfo &info, const DArray<const char *> requiredExtensions)
{
	const Version &appVersion = info.appVersion;
	const Version &engineVersion = info.engineVersion;

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = info.appName;	
	appInfo.applicationVersion = VK_MAKE_VERSION(appVersion.major, appVersion.major, appVersion.minor);
	appInfo.pEngineName = "AGEngine";
	appInfo.engineVersion = VK_MAKE_VERSION(engineVersion.major, engineVersion.minor, engineVersion.patch);
	appInfo.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<u32>(requiredExtensions.count());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

#ifdef AGE_DEBUG
	createInfo.enabledLayerCount = static_cast<u32>(k_validationLayers.size());
	createInfo.ppEnabledLayerNames = k_validationLayers.data();
#else
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = nullptr;
#endif

	AGE_VK_CHECK(vkCreateInstance(&createInfo, context.allocator, &context.instance));
	age_log(k_tag, "Created Instance.");
}



#ifdef AGE_DEBUG
void createDebugMessenger(Context &context)
{
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

	auto f_createDebugUtilsMessengerEXT = AGE_VK_GET_COMMAND(context.instance, vkCreateDebugUtilsMessengerEXT);
	if (f_createDebugUtilsMessengerEXT != nullptr) {
		AGE_VK_CHECK(f_createDebugUtilsMessengerEXT(context.instance, &createInfo, context.allocator, &context.debugMessenger));
	} else {
		age_error(k_tag, "Unable to create DebugUtilsMessenger due to extension not being present.");
	}
}
#endif	// AGE_DEBUG



void createDevice(Context &context, const DArray<const char *> &extensions)
{
	DArray<VkDeviceQueueCreateInfo> queueCreateInfoArray(static_cast<u32>(EQueueFamily::Count));

	float queuePriority = 1.0f;
	for (int i = 0; i < static_cast<u32>(EQueueFamily::Count); i++) {
		VkDeviceQueueCreateInfo queueCreateInfo;
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pNext = nullptr;
		queueCreateInfo.flags = 0;
		queueCreateInfo.queueFamilyIndex = context.queueIndices[i];
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
	createInfo.enabledExtensionCount = static_cast<u32>(extensions.count());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// For older versions of Vulkan, it might be necessary to also set the validation layers here.
	// Since this is not necessary at the time of this implementation, let's skip that.
	createInfo.enabledLayerCount = 0;
	
	AGE_VK_CHECK(vkCreateDevice(context.physicalDevice, &createInfo, context.allocator, &context.device));
	age_log(k_tag, "Created logical device.");
}



void createCommandPools(Context &context)
{
	{	// Create Graphics Command Pool
		VkCommandPoolCreateInfo commandPoolInfo = {};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.queueFamilyIndex = context.queueIndices[static_cast<i32>(EQueueFamily::Graphics)];

		AGE_VK_CHECK(vkCreateCommandPool(context.device, &commandPoolInfo, context.allocator, &context.graphicsCommandPool));

		age_log(k_tag, "Created Graphics Command Pool");
	}


	{	// Create Transfer Command Pool
		VkCommandPoolCreateInfo commandPoolInfo = {};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		commandPoolInfo.queueFamilyIndex = context.queueIndices[static_cast<i32>(EQueueFamily::Transfer)];

		AGE_VK_CHECK(vkCreateCommandPool(context.device, &commandPoolInfo, context.allocator, &context.transferCommandPool));

		age_log(k_tag, "Created Transfer Command Pool");
	}
}



Context createContext(const ContextCreateInfo &info)
{
	Context context = {};
	context.allocator = info.allocator;

	const auto extensions = getRequiredExtensions();
	createInstance(context, info, extensions);
	IF_DEBUG(createDebugMessenger(context));

	{	// Create Surface
		AGE_VK_CHECK(glfwCreateWindowSurface(context.instance, info.window, context.allocator, &context.surface));
		age_log(k_tag, "Created Window Surface.");
	}

	selectPhysicalDevice(context, extensions);
	createDevice(context, extensions);
	createCommandPools(context);

	{	// Store Queue Handles
		for (int i = 0; i < k_queueFamilyCount; i++)
			vkGetDeviceQueue(context.device, context.queueIndices[i], 0, &context.queues[i]);
	}

	return context;
}



void destroyContext(Context &context)
{
	vkDestroyCommandPool(context.device, context.transferCommandPool, context.allocator);
	vkDestroyCommandPool(context.device, context.graphicsCommandPool, context.allocator);

	vkDestroyDevice(context.device, context.allocator);

#ifdef AGE_DEBUG
	{	// DestroyDebugUtilsMessenger
		auto destroyDebugUtilsMessengerEXT = AGE_VK_GET_COMMAND(context.instance, vkDestroyDebugUtilsMessengerEXT);
		if (destroyDebugUtilsMessengerEXT != nullptr)
			destroyDebugUtilsMessengerEXT(context.instance, context.debugMessenger, context.allocator);
	}
#endif

	vkDestroyInstance(context.instance, context.allocator);
}

}	// namespace age::vk