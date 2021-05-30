#include "Game.h"

#include <Core/Log/Log.h>

#include "AGE/Vendor/GLFW.hpp"
#include "AGE/Renderer/Vulkan/VulkanSystem.h"


namespace age
{

constexpr char k_tag[] = "Game";

void Game::Run()
{
	logger::enable("Game");

	{	// initEngineSystems
		glfwInit();

		WindowInfo windowInfo = GetWindowInfo();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		_window = glfwCreateWindow(windowInfo.size.w, windowInfo.size.h, windowInfo.title.c_str(), nullptr, nullptr);

		vk::VulkanSystem::get().init(_window);
	}

	g_log(k_tag, "Initializing Game.");
	init();

	g_log(k_tag, "Starting Game Loop.");
	while(!glfwWindowShouldClose(_window)) {
		glfwPollEvents();
		update();
	}

	g_log(k_tag, "Starting Game Cleanup.");
	cleanup();

	{	// cleanupEngineSystems
		vk::VulkanSystem::get().cleanup();

		glfwDestroyWindow(_window);
		glfwTerminate();
	}
}

}	// namespace age