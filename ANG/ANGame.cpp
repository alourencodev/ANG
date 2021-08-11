#include "ANG/ANGame.h"

DEFINE_GAME(ANGame);

void ANGame::init()
{
}

void ANGame::update()
{
}

void ANGame::cleanup()
{
}

age::Game::WindowInfo ANGame::GetWindowInfo()
{
	return {"ANGame", {1920, 1080}};
}
