#pragma once

#include <vulkan/vulkan.h>

#include <Core/Attributes.hpp>
#include <Core/BitField.hpp>
#include <Core/BuildScheme.hpp>
#include <Core/DArray.hpp>
#include <Core/SArray.hpp>


namespace age::vk
{

class VulkanSystem;

enum class e_QueueFamily : u8
{
	Graphics,

	Count
};


class QueueIndices
{
public:
	QueueIndices() = default;

	u32 get(e_QueueFamily queueFamily) const
	{
		const u8 queueFamilyIndex = static_cast<u8>(queueFamily);
		g_assert(_indexMap.isSet(queueFamilyIndex), "Can't find queue family(%d)", queueFamilyIndex);
		return _indices[queueFamilyIndex];
	}

private:
	using QueueIndexArray = SArray<u32, static_cast<u8>(e_QueueFamily::Count)>;
	friend class VulkanSystem;

	BitField _indexMap;
	QueueIndexArray _indices = {};

	void set(e_QueueFamily queueFamily, u32 index) 
	{
		const u8 queueFamilyIndex = static_cast<u8>(queueFamily);
		_indexMap.set(queueFamilyIndex);
		_indices[static_cast<u8>(queueFamily)] = index;
	}
};


class VulkanSystem
{
public:

	static VulkanSystem &get() { static VulkanSystem instance; return instance; } ;
	
	void init();
	void cleanup();

private:
	VulkanSystem() = default;
	~VulkanSystem() = default;

	VkPhysicalDevice pickPhysicalDevice(const DArray<VkPhysicalDevice> &candidates) const;
	QueueIndices getDeviceQueueIndices(VkPhysicalDevice physicalDevice) const;
	bool isDeviceCompatible(VkPhysicalDevice physicalDevice) const;

	VkInstance _instance = VK_NULL_HANDLE;
	VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;
	VkDevice _device = VK_NULL_HANDLE;
	QueueIndices _queueIndices;

#ifdef _RELEASE_SYMB
	VkDebugUtilsMessengerEXT _debugMessenger;
#endif
};

}
