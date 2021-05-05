#pragma once

#include "AGEAPI.hpp"

#include <Core/Math/Vec2.hpp>

struct GLFWwindow;

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

	GLFWwindow *GetWindow() { return _window; }

protected:
	struct WindowInfo
	{
		std::string title;
		sizei size;
	};

	virtual WindowInfo GetWindowInfo() = 0;

private:
	GLFWwindow *_window;
};

}

#define DEFINE_GAME(GAME)			\
static age::Game *g_createGame()	\
{									\
	return new GAME##();			\
}
