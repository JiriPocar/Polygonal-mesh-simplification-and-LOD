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
#include "rendering/FrameBuffer.hpp"
#include "rendering/CommandManager.hpp"
#include "rendering/Renderer.hpp"
#include "resources/Model.hpp"
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

		FrameBuffer framebuffer(device, renderPass, swapchain);
		std::cout << "\nFramebuffers created successfully!" << std::endl;
		for (size_t i = 0; i < framebuffer.getFramebuffers().size(); i++) {
			std::cout << "  - Framebuffer " << i << ": "
				<< (framebuffer.getFrameBufferAt(i) ? "VALID" : "INVALID") << std::endl;
		}

		std::cout << "\nCreating command manager..." << std::endl;
		CommandManager commandManager(device);
		commandManager.createCommandBuffers(static_cast<uint32_t>(swapchain.getImages().size()));
		std::cout << "Amount of command buffers: " << swapchain.getImages().size() << std::endl;

		std::cout << "\nLoading model..." << std::endl;
		Model model(device, "C:/Users/tf2ma/source/repos/Renderer/assets/Fox.gltf");
		std::cout << "Model loaded successfully!" << std::endl;

		std::cout << "\nCreating renderer..." << std::endl;
		Renderer renderer(device, swapchain, renderPass, pipeline,
						  framebuffer, commandManager, window, surface.get(), model);

		while (!window.shouldClose()) {
			window.pollEvents();
			if (glfwGetKey(window.getGLFWWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
				break;
			}
			try {
				renderer.drawFrame();
			}
			catch (const vk::OutOfDateKHRError&) {
				renderer.recreateSwapchain();
			}
			catch (const std::exception& e) {
				std::cerr << "Error during frame rendering: " << e.what() << std::endl;
			}
			
			if (window.wasResized()) {
				window.resetResizedFlag();
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