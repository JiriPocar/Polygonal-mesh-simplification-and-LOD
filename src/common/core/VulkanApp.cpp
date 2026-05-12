/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file VulkanApp.cpp
 * @brief Base class for Vulkan applications.
 *
 * This file implements the VulkanApp class for managing the Vulkan apps,
 * including initialization, main loop, and cleanup. Common interface is
 * provided to implement specific application logic further.
 */

#include "VulkanApp.hpp"
#include <iostream>

VulkanApp::VulkanApp(int width, int height, bool enableVsync, const char* appName)
	: window(width, height, appName),
	  instance(false),
	  surface(window.createSurface(instance)),
	  device(instance, *surface),
	  swapchain(device, *surface, enableVsync, window.getWidth(), window.getHeight()),
	  renderPass(device, swapchain.getImageFormat()),
	  uniformBuffer(device),
	  descriptor(device, uniformBuffer),
	  commandManager(device),
	  framebuffer(device, renderPass, swapchain),
	  ui(instance, device, swapchain, renderPass, window, commandManager)
{
	commandManager.createCommandBuffers(static_cast<uint32_t>(swapchain.getImages().size()));

	camera.setPerspective(45.0f, swapchain.getExtent().width / (float)swapchain.getExtent().height, 0.1f, 1000.0f);
	camera.setView(
		glm::vec3(0.0f, 5.0f, 25.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	// set mouse callback  
	window.setMouseCallback([this](double xPos, double yPos) {
		ui.handleMouseMove(xPos, yPos);
		if (cameraActive)
		{
			camera.handleMouseInput(static_cast<float>(xPos), static_cast<float>(yPos));
		}
	});
}

void VulkanApp::run()
{
	init();

	auto last = std::chrono::high_resolution_clock::now();
	bool pressedDisableUI = false;
	while (!window.shouldClose())
	{
		auto current = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(current - last).count();
		last = current;

		window.pollEvents();

		if (glfwGetKey(window.getGLFWWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			break;
		}

		if (glfwGetKey(window.getGLFWWindow(), GLFW_KEY_M) == GLFW_PRESS)
		{
			window.disableCursor();
			cameraActive = true;
		}

		if (glfwGetKey(window.getGLFWWindow(), GLFW_KEY_N) == GLFW_PRESS)
		{
			window.enableCursor();
			cameraActive = false;
			camera.resetMouse();
		}

		bool isUPressed = glfwGetKey(window.getGLFWWindow(), GLFW_KEY_U) == GLFW_PRESS;
		if (isUPressed && !pressedDisableUI) {
			showUI = !showUI;
		}
		pressedDisableUI = isUPressed;

		camera.handleInput(window.getGLFWWindow(), deltaTime);

		update(deltaTime);
		drawFrame();
	}

	// wait for gpu to finish before exiting
	device.operator*().waitIdle();
}

/* End of the VulkanApp.cpp file */