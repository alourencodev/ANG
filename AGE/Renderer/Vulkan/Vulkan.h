#pragma once

struct GLFWwindow;

namespace age::vk
{

void init(GLFWwindow *window);
void cleanup();

} // namespace age::vk