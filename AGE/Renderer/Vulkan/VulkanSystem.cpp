#include "VulkanSystem.h"

#include <cstring>

#include <Core/DArray.hpp>
#include <Core/Meta.hpp>
#include <Core/Types.hpp>

#include "AGE/Vendor/GLFW.hpp"
#include "AGE/Renderer/Vulkan/VulkanUtils.hpp"


namespace age
{

constexpr char k_tag[] = "VulkanSystem";

static const DArray<const char *> k_validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

void VulkanSystem::init()
{
	// Check if validation layers are available
	if constexpr(meta::isDebugBuild::value)
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
		appInfo.pApplicationName = "TEST Name";				// TODO
		appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);	// TODO 
		appInfo.pEngineName = "AGEngine";
		appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
		appInfo.apiVersion = VK_API_VERSION_1_2;

		u32 glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;

		if constexpr (meta::isDebugBuild::value) {
			createInfo.enabledLayerCount = static_cast<u32>(k_validationLayers.count());
			createInfo.ppEnabledLayerNames = k_validationLayers.data();
		} else {
			createInfo.enabledLayerCount = 0;
		}

		AGE_VK_CHECK(vkCreateInstance(&createInfo, nullptr, &_instance));		
	}
}

void VulkanSystem::cleanup()
{
	vkDestroyInstance(_instance, nullptr);
}

}