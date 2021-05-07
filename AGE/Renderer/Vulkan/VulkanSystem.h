#pragma once

#include <vulkan/vulkan.h>

namespace age
{

class VulkanSystem
{
public:
	static VulkanSystem &get() { static VulkanSystem instance; return instance; } ;
	
	void init();
	void cleanup();

private:
	VulkanSystem() = default;
	~VulkanSystem() = default;

	VkInstance _instance;
};

}
