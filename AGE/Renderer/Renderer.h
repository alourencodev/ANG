#pragma once

struct GLFWwindow;

namespace age
{

class Renderer
{
public:
	static Renderer s_inst;

	void init(GLFWwindow *window);
	void update();
	void cleanup();
};

}
