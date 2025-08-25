#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

class Window
{
public:
	Window(int width, int height, const char* name);
	~Window();

	bool shouldClose() const
	{
		return glfwWindowShouldClose(window);
	}

	void pollEvents() const
	{
		glfwPollEvents();
	}

	GLFWwindow* getGLFWWindow() const { return window; }

	vk::UniqueSurfaceKHR createSurface(vk::Instance instance);

private:
	GLFWwindow* window;
};