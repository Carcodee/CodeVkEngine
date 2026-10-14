//

// Created by carlo on 2024-10-02.
//

#ifndef WINDOW_PROVIDER_HPP
#define WINDOW_PROVIDER_HPP
#include <glm/vec2.hpp>

class window_provider
{
  public:
	window_provider(int width, int height, const char *name)
	{
		current_width       = width;
		current_height      = height;
		this->name         = name;
		framebuffer_resized = false;
	}

	void init_glfw()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		window = glfwCreateWindow(current_width, current_height, name, nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetWindowSizeCallback(window, framebuffer_resize_callback_glfw);
	}

	bool window_should_close()
	{
		should_close = glfwWindowShouldClose(window);
		return should_close;
	}

	void destroy_window()
	{
		glfwDestroyWindow(window);
	}
	void terminate()
	{
		glfwTerminate();
	}
	void poll_events()
	{
		glfwPollEvents();
	}
	double get_time()
	{
		return glfwGetTime();
	}
	glm::uvec2 get_window_size()
	{
		return glm::uvec2(current_width, current_height);
	}

	GLFWwindow *window;
	bool        framebuffer_resized;
	int         current_width;
	int         current_height;
	const char *name;

	static void framebuffer_resize_callback_glfw(GLFWwindow *window, int width, int height)
	{
		auto instance                = reinterpret_cast<::window_provider *>(glfwGetWindowUserPointer(window));
		instance->current_width       = width;
		instance->current_height      = height;
		instance->framebuffer_resized = true;
	}

  private:
	bool should_close;
};
#endif        // WINDOW_PROVIDER_HPP
