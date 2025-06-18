//

// Created by carlo on 2024-10-02.
//

#ifndef WINDOW_HPP
#define WINDOW_HPP
#include <glm/vec2.hpp>

class WindowProvider
{
  public:
	WindowProvider(int width, int height, const char *name)
	{
		currentWidth       = width;
		currentHeight      = height;
		this->name         = name;
		framebufferResized = false;
	}

	void InitGlfw()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		window = glfwCreateWindow(currentWidth, currentHeight, name, nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetWindowSizeCallback(window, FramebufferResizeCallback_GLFW);
	}

	bool WindowShouldClose()
	{
		windowShouldClose = glfwWindowShouldClose(window);
		return windowShouldClose;
	}

	void DestroyWindow()
	{
		glfwDestroyWindow(window);
	}
	void Terminate()
	{
		glfwTerminate();
	}
	void PollEvents()
	{
		glfwPollEvents();
	}
	double GetTime()
	{
		return glfwGetTime();
	}
	glm::uvec2 GetWindowSize()
	{
		return glm::uvec2(currentWidth, currentHeight);
	}

	GLFWwindow *window;
	bool        framebufferResized;
	int         currentWidth;
	int         currentHeight;
	const char *name;

	static void FramebufferResizeCallback_GLFW(GLFWwindow *window, int width, int height)
	{
		auto instance                = reinterpret_cast<WindowProvider *>(glfwGetWindowUserPointer(window));
		instance->currentWidth       = width;
		instance->currentHeight      = height;
		instance->framebufferResized = true;
	}

  private:
	bool windowShouldClose;
};
#endif        // WINDOW_HPP
