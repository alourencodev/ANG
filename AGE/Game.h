#pragma once

#include <Core/Math/Vec2.hpp>

#include "AGEAPI.hpp"

struct GLFWwindow;

namespace age
{

class AGE_API Game
{
public:
	Game() = default;
	virtual ~Game() = default;
	
	void Run();

	GLFWwindow *GetWindow() { return _window; }

protected:
	struct WindowInfo
	{
		std::string title;
		math::sizei size;
	};

	virtual void init() = 0; 
	virtual void update() = 0;
	virtual void cleanup() = 0;

	virtual WindowInfo GetWindowInfo() = 0;

private:
	GLFWwindow *_window = nullptr;
};

static Game *s_game = nullptr;

}    // namespace age

#define DEFINE_GAME(GAME)	\
age::Game *s_createGame()	\
{							\
	return new GAME;		\
}
