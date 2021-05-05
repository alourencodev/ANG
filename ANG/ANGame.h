#pragma once

#include <AGE/AGE.hpp>


class ANGame : public age::Game
{
public:
	void init() override final;
	void update() override final;
	void cleanup() override final;

private:
	WindowInfo GetWindowInfo() override final;
};
