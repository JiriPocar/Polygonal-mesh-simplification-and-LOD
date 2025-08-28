#include "window.h"
#include <stdexcept>
#include <iostream>

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

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

Window::~Window()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
	auto *win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	win->width = width;
	win->height = height;
	win->resized = true;

	if (win->resizeCallback)
	{
		win->resizeCallback(width, height);
	}

	//std::cout << "Window resized to " << width << "x" << height << std::endl;
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