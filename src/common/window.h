/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file window.hpp
 * @brief Window management using GLFW and Vulkan surface creation.
 * 
 * This file implements the Window class, which is responsible for creating
 * and managing a window using GLFW, handling window resize events, and creating
 * a Vulkan surface for rendering. The class provides methods to check if the
 * window should close, poll events, and manage mouse input.
 */

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

	// getters and setters
	GLFWwindow* getGLFWWindow() const { return window; }
	int getWidth() const { return width; }
	int getHeight() const { return height; }

	bool wasResized() const { return resized; };
	void resetResizedFlag() { resized = false; }
	void setResizeCallback(ResizeCallback callback) { resizeCallback = callback; };

	void setMouseCallback(std::function<void(double, double)> callback);
	void disableCursor();
	void enableCursor();

	/**
	* @brief Creates a Vulkan surface for the window using the Vulkan instance.
	* 
	* @param instance The Vulkan instance to use for creating the surface
	* 
	* @return A unique handle to the created Vulkan surface
	*/
	vk::UniqueSurfaceKHR createSurface(vk::Instance instance);

private:
	/**
	* @brief Static callback function for handling framebuffer resize events.
	* 
	* @param window GLFW window that was resized
	* @param width The new width of the framebuffer
	* @param height The new height of the framebuffer
	*/
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

	/**
	* @brief Static callback function for handling mouse movement events.
	* 
	* @param window The GLFW window that received the mouse movement event
	* @param xpos The new x coord of the mouse cursor
	* @param ypos The new y coord of the mouse cursor
	*/
	static void mouseCallback(GLFWwindow* window, double xpos, double ypos);

	// callback function
	std::function<void(double, double)> mouseMoveCallback;

	int width = 1800;
	int height = 920;

	GLFWwindow* window;
	bool resized = false;
	ResizeCallback resizeCallback;
};

/* End of the window.hpp file */