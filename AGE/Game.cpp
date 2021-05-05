#include "Game.h"

#include <Core/Log/Log.h>

#include "Vendor/GLFW.hpp"


namespace age
{

constexpr char k_tag[] = "Game";

void Game::Run()
{
	glfwInit();
	_window = glfwCreateWindow(700, 400, "Test Title", nullptr, nullptr);

	g_log(k_tag, "Initializing Game.");
	init();

	g_log(k_tag, "Starting Game Loop.");
	while(!glfwWindowShouldClose(_window))
		update();

	g_log(k_tag, "Starting Game Cleanup.");
	cleanup();

	glfwDestroyWindow(_window);
	glfwTerminate();
}

}