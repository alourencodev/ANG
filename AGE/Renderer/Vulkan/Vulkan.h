#pragma once

#include <vulkan/vulkan.h>

#include <Core/DArray.hpp>


struct GLFWwindow;

namespace age::vk
{

void init(GLFWwindow *window);
void cleanup();
void recreateRenderEnvironment();
void draw(const DArray<VkCommandBuffer> &commandBuffers);

} // namespace age::vk