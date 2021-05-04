#pragma once

#include "AGEAPI.hpp"


namespace age
{

class AGE_API Game
{
public:
	Game() = default;
	virtual ~Game() = default;

	virtual void init() = 0; 
	virtual void update() = 0;
	virtual void cleanup() = 0;

	virtual void Run() final;
};

}

#define DEFINE_GAME(GAME)			\
static age::Game *g_createGame()	\
{									\
	return new GAME##();			\
}
