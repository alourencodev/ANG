#pragma once

#include "Game.h"

extern age::Game *s_createGame();

int main(int argc, char *argv[])
{
	age::s_game = s_createGame();
	age::s_game->Run(argc, argv);
	delete age::s_game;

	return 0;
}
