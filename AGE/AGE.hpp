#pragma once

#include "Game.h"

extern age::Game *g_createGame();

int main(int argc, char **argv)
{
	age::Game *game = g_createGame();
	game->Run();
	delete game;
	return 0;
}
