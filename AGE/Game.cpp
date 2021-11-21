#include "Game.h"

#include <Core/BuildScheme.hpp>
#include <Core/Log/Log.h>
#include <Core/StackArray.hpp>

#include "AGE/Vendor/GLFW.hpp"
#include "AGE/Renderer/Renderer.h"
#include "AGE/Systems/CommandSystem.h"

#ifndef AGE_RELEASE
#include <cstdio>
#endif

namespace age
{

constexpr char k_tag[] = "Game";
constexpr u8 k_maxTitleLength = 128;

#ifdef AGE_RELEASE_DBG_INFO
void runGameArguments(int argc, char *argv[])
{
	const char *command = nullptr;
	CommandSystem::CommandArgs commandArgs;

	auto runCommand = [](const char * command, CommandSystem::CommandArgs &args) -> void
	{
		bool wasCommandFound = s_commandSystem.runCommand(command, args);
		args.clear();
		age_assert(wasCommandFound, "Command %s could not be found.", command);
	};

	for (int i = 1; i < argc; i++)
	{
		// arguments that start with '-' are commands, otherwise they are arguments
		if (argv[i][0] == '-') {
			if (command != nullptr)
				runCommand(command, commandArgs);

			command = argv[i];
		}
		else {
			age_assert(command != nullptr, "Invalid command %s. Every command should start with '-'.", argv[i]);
			commandArgs.add(argv[i]);
		}
	}

	if (command != nullptr)
		runCommand(command, commandArgs);
}
#endif


static void onFramebufferResize(GLFWwindow *window, int width, int height)
{
	Renderer::s_inst.onWindowResize(window, width, height);
}

void Game::Run(int argc, char *argv[])
{
#ifdef AGE_RELEASE_DBG_INFO
	// Get Commands ready as soon as possible
	s_commandSystem.init();
	runGameArguments(argc, argv);
#endif

	age_log(k_tag, "Initializing Engine Systems");
	{	// initEngineSystems
		glfwInit();

		WindowInfo windowInfo = GetWindowInfo();

#ifdef AGE_RELEASE
		const char *title = windowInfo.title.c_str();
#else
		char title[k_maxTitleLength];
		snprintf(title, sizeof(title), "%s [%s]", windowInfo.title.c_str(), BUILD_SCHEME_STR);
#endif

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		_window = glfwCreateWindow(windowInfo.size.w, windowInfo.size.h, title, nullptr, nullptr);

		Renderer::s_inst.init(_window);
		glfwSetFramebufferSizeCallback(_window, onFramebufferResize);
	}

	age_log(k_tag, "Initializing Game.");
	init();

	age_log(k_tag, "Starting Game Loop.");
	while(!glfwWindowShouldClose(_window)) {
		glfwPollEvents();
		update();
		Renderer::s_inst.update();
	}

	age_log(k_tag, "Starting Game Cleanup.");
	cleanup();

	age_log(k_tag, "Starting Engine Systems Cleanup.");
	{	// cleanupEngineSystems
		Renderer::s_inst.cleanup();

		glfwDestroyWindow(_window);
		glfwTerminate();
	}
}

}	// namespace age
