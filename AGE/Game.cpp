#include "Game.h"

#include <Core/Log/Log.h>


namespace age
{

constexpr char k_tag[] = "Game";

void Game::Run()
{
	g_log(k_tag, "Initializing Game.");
	init();

	// TODO: Loop based on window
	g_log(k_tag, "Starting Game Loop.");
	while(true)
		update();

	g_log(k_tag, "Starting Game Cleanup.");
	cleanup();
}

}