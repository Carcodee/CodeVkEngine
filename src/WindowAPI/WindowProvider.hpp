//

// Created by carlo on 2024-10-02.
//

#ifndef WINDOW_PROVIDER_HPP
#define WINDOW_PROVIDER_HPP
#include <glm/vec2.hpp>

class Window_Provider
{
  public:
	Window_Provider(int width, int height, const char *name)
	{
		current_width       = width;
		current_height      = height;
		this->name         = name;
		framebuffer_resized = false;
	}

	void Init_Glfw()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
		window = glfwCreateWindow(current_width, current_height, name, nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetWindowSizeCallback(window, Framebuffer_Resize_Callback_Glfw);
	}

	bool Window_Should_Close()
	{
		should_close = glfwWindowShouldClose(window);
		return should_close;
	}

	void Destroy_Window()
	{
		glfwDestroyWindow(window);
	}
	void Terminate()
	{
		glfwTerminate();
	}
	void Poll_Events()
	{
		glfwPollEvents();
	}
	double Get_Time()
	{
		return glfwGetTime();
	}
	glm::uvec2 Get_Window_Size()
	{
		return glm::uvec2(current_width, current_height);
	}

	GLFWwindow *window;
	bool        framebuffer_resized;
	int         current_width;
	int         current_height;
	const char *name;

	static void Framebuffer_Resize_Callback_Glfw(GLFWwindow *window, int width, int height)
	{
		auto instance                = reinterpret_cast<::Window_Provider *>(glfwGetWindowUserPointer(window));
		instance->current_width       = width;
		instance->current_height      = height;
		instance->framebuffer_resized = true;
	}

  private:
	bool should_close;
};
#endif        // WINDOW_PROVIDER_HPP
