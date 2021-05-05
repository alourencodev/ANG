#pragma once

#include "Game.h"

extern age::Game *g_createGame();


int main(int argc, char **argv)
{
	age::g_game = g_createGame();
	age::g_game->Run();
	delete age::g_game;

	return 0;
}
