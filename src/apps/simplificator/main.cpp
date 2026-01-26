/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file main.cpp
 * @brief Main entry point for the Vulkan application.
 *
 * This file contains the main function that initializes the Vulkan application.
 */

#include <vulkan/vulkan.hpp>
#include <iostream>
#include <chrono>
#include "core/Instance.hpp"
#include "core/Device.hpp"
#include "core/Swapchain.hpp"
#include "core/Pipeline.hpp"
#include "rendering/RenderPass.hpp"
#include "rendering/FrameBuffer.hpp"
#include "rendering/CommandManager.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/Descriptors.hpp"
#include "rendering/UniformBuffer.hpp"
#include "resources/Model.hpp"
#include "resources/DualModel.hpp"
#include "scene/Camera.hpp"
#include "scene/Transform.hpp"
#include "ui/ui.hpp"
#include "window.h"

int main() {
	try
	{
		std::cout << "Starting Vulkan application...\n" << std::endl;

		std::cout << "\nCreating window..." << std::endl;
		Window window(1400, 800, "Vulkan Window");
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
		Swapchain swapchain(device, *surface, 1200, 1000);

		std::cout << "\nSwapchain created successfully!" << std::endl;
		std::cout << "  - Image format: " << vk::to_string(swapchain.getImageFormat()) << std::endl;
		std::cout << "  - Extent: " << swapchain.getExtent().width << "x" << swapchain.getExtent().height << std::endl;
		std::cout << "  - Image count: " << swapchain.getImages().size() << std::endl;
		std::cout << "  - Image views: " << swapchain.getImageViews().size() << std::endl;

		RenderPass renderPass(device, swapchain.getImageFormat());
		std::cout << "\nRenderPass format: " << vk::to_string(renderPass.getFormat()) << std::endl;

		std::cout << "Creating uniform buffer..." << std::endl;
		UniformBuffer uniformBuffer(device);
		std::cout << "Uniform buffer created successfully!" << std::endl;

		std::cout << "Creating descriptor set..." << std::endl;
		Descriptor descriptorSet(device, uniformBuffer);
		std::cout << "Descriptor set created successfully!" << std::endl;

		Pipeline pipeline(device, renderPass, swapchain.getExtent(), descriptorSet.getLayout());
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

		std::cout << "\nLoading initial model..." << std::endl;
		//std::unique_ptr<Model> currentModel = std::make_unique<Model>(device, "../../../assets/Lantern.gltf");
		std::unique_ptr<DualModel> currentDualModel = std::make_unique<DualModel>(device, "assets/Fox.gltf");
		std::cout << "Model loaded successfully!" << std::endl;

		std::cout << "\nUI initialization" << std::endl;
		UserInterface ui(instance, device, swapchain, renderPass, window, commandManager);
		std::cout << "UI initialized successfully!" << std::endl;

		Camera camera;
		camera.setPerspective(45.0f, swapchain.getExtent().width / (float)swapchain.getExtent().height, 0.1f, 1000.0f);
		camera.setView(glm::vec3(0.0f, 0.5f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		Transform transform;
		transform.setPos(glm::vec3(0.0f, 0.0f, 0.0f));
		transform.setRot(glm::vec3(0.0f, 0.0f, 0.0f));
		// get model auto scale and apply
		float modelScale = currentDualModel->getOriginalModel().getScaleIndex();
		transform.setScale(glm::vec3(modelScale, modelScale, modelScale));

		std::cout << "\nCreating renderer..." << std::endl;
		Renderer renderer(device, swapchain, renderPass, pipeline,
						  framebuffer, commandManager, window, surface.get(), currentDualModel->getOriginalModel(), uniformBuffer, descriptorSet);
		renderer.setDualModel(*currentDualModel);
		auto last = std::chrono::high_resolution_clock::now();
		float xRotation = 0.0f;
		float yRotation = 0.0f;
		float zRotation = 0.0f;

		// fps camera movement
		bool cameraActive = false;
		window.setMouseCallback([&](double xPos, double yPos)
		{
				ui.handleMouseMove(xPos, yPos);

				if (cameraActive)
				{
					camera.handleMouseInput(xPos, yPos, cameraActive);
				}
		});

		bool rotate = true;

		while (!window.shouldClose())
		{
			auto current = std::chrono::high_resolution_clock::now();
			float delta = std::chrono::duration<float, std::chrono::seconds::period>(current - last).count();
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

			camera.handleInput(window.getGLFWWindow(), delta);

			Transform t = ui.fetchTransform();
			xRotation = t.getRot().x;
			yRotation = t.getRot().y;
			zRotation = t.getRot().z;

			auto rotationAxes = ui.getRotationAxes();

			rotate = (rotationAxes[0] || rotationAxes[1] || rotationAxes[2]);

			if (rotate)
			{
				if (rotationAxes[0] == true)
				{
					xRotation += 50.0f * delta;
					if (xRotation >= 360.0f) xRotation -= 360.0f;
				}

				if (rotationAxes[1] == true)
				{
					yRotation += 50.0f * delta;
					if (yRotation >= 360.0f) yRotation -= 360.0f;
				}

				if (rotationAxes[2] == true)
				{
					zRotation += 50.0f * delta;
					if (zRotation >= 360.0f) zRotation -= 360.0f;
				}
			}

			transform.setRot(glm::vec3(xRotation, yRotation, zRotation));
			ui.setTransform(transform);

			ui.beginFrame(currentDualModel, device, renderer, transform);

			try {
				//renderer.drawFrame(camera, transform, ui);
				renderer.drawSplitScreen(camera, transform, ui);
			}
			catch (const vk::OutOfDateKHRError&) {
				renderer.recreateSwapchain();
			}
			catch (const std::exception& e) {
				std::cerr << "Error during frame rendering: " << e.what() << std::endl;
			}
			
			if (window.wasResized()) {
				window.resetResizedFlag();
				camera.setPerspective(45.0f, swapchain.getExtent().width / (float)swapchain.getExtent().height, 0.1f, 1000.0f);
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