#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <functional>

class Window
{
public:
	using ResizeCallback = std::function<void(int, int)>;

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
	int getWidth() const { return width; }
	int getHeight() const { return height; }

	bool wasResized() const { return resized; };
	void resetResizedFlag() { resized = false; }
	void setResizeCallback(ResizeCallback callback) { resizeCallback = callback; };

	void setMouseCallback(std::function<void(double, double)> callback);
	void disableCursor();
	void enableCursor();

	vk::UniqueSurfaceKHR createSurface(vk::Instance instance);

private:
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
	std::function<void(double, double)> mouseMoveCallback;

	int width;
	int height;

	GLFWwindow* window;
	bool resized = false;
	ResizeCallback resizeCallback;
};