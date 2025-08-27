/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file main.cpp
 * @brief Main entry point for the Vulkan application.
 *
 * This file contains the main function that initializes the Vulkan application.
 */

#include <vulkan/vulkan.hpp>
#include <iostream>
#include "core/Instance.hpp"
#include "core/Device.hpp"
#include "core/Swapchain.hpp"
#include "core/Pipeline.hpp"
#include "rendering/RenderPass.hpp"
#include "window.h"

int main() {
	try
	{
		std::cout << "Starting Vulkan application...\n" << std::endl;

		std::cout << "\nCreating window..." << std::endl;
		Window window(800, 600, "Vulkan Window");
		std::cout << "Successfully created window!" << std::endl;

		std::cout << "\nCreating instance..." << std::endl;
		Instance instance(false);
		std::cout << "Successfully created instance!" << std::endl;

		std::cout << "\nGetting surface..." << std::endl;
		auto surface = window.createSurface(instance);
		std::cout << "Successfully got surface!" << std::endl;

		std::cout << "\nCreating device..." << std::endl;
		Device device(instance, *surface);
		std::cout << "Successfully created device!" << std::endl;

		std::cout << "\nCreating swapchain..." << std::endl;
		Swapchain swapchain(device, *surface, 800, 600);

		std::cout << "\nSwapchain created successfully!" << std::endl;
		std::cout << "  - Image format: " << vk::to_string(swapchain.getImageFormat()) << std::endl;
		std::cout << "  - Extent: " << swapchain.getExtent().width << "x" << swapchain.getExtent().height << std::endl;
		std::cout << "  - Image count: " << swapchain.getImages().size() << std::endl;
		std::cout << "  - Image views: " << swapchain.getImageViews().size() << std::endl;

		RenderPass renderPass(device, swapchain.getImageFormat());
		std::cout << "\nRenderPass format: " << vk::to_string(renderPass.getFormat()) << std::endl;

		Pipeline pipeline(device, renderPass, swapchain.getExtent());
		std::cout << "\nPipeline created successfully!" << std::endl;
		std::cout << "  - Pipeline layout: " << (pipeline.getLayout() ? "VALID" : "INVALID") << std::endl;
		std::cout << "  - Pipeline handle: " << (pipeline.get() ? "VALID" : "INVALID") << std::endl;

		while (!window.shouldClose()) {
			window.pollEvents();
			if (glfwGetKey(window.getGLFWWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
				break;
			}
		}
	}
	catch (vk::Error& e)
	{
		std::cout << "Caught a Vulkan error" << e.what() << std::endl;
	}
	catch (std::exception& e)
	{
		std::cout << "Caught a standard exception: " << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "Caught an unknown exception" << std::endl;
	}
}

/* End of the main.cpp file */