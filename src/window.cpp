#include "window.h"
#include <stdexcept>

Window::Window(int width, int height, const char* name)
{
	if (!glfwInit())
	{
		throw std::runtime_error("Failed to initialize GLFW");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(width, height, name, nullptr, nullptr);

	if (!window)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window");
	}
}

Window::~Window()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

vk::UniqueSurfaceKHR Window::createSurface(vk::Instance instance)
{
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface");
	}
	return vk::UniqueSurfaceKHR(surface, instance);
}