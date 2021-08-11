#pragma once

#include "Game.h"

#include <iostream>

extern age::Game *s_createGame();


int main(int argc, char *argv[])
{
#ifdef AGE_DEBUG
	// TODO: Parse command line parameters
	for (int i = 1; i < argc; i++)
		std::cout << "ARG" << argv[i] << std::endl;
#endif

	age::s_game = s_createGame();
	age::s_game->Run();
	delete age::s_game;

	return 0;
}
