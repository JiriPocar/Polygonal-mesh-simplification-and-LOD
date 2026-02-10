#include <vulkan/vulkan.hpp>
#include <iostream>
#include <chrono>
#include <vector>
#include "core/Instance.hpp"
#include "core/Device.hpp"
#include "core/Swapchain.hpp"
#include "core/SpiralPipeline.hpp"
#include "rendering/RenderPass.hpp"
#include "rendering/FrameBuffer.hpp"
#include "rendering/CommandManager.hpp"
#include "rendering/SpiralRenderer.hpp"
#include "rendering/Descriptors.hpp"
#include "rendering/UniformBuffer.hpp"
#include "resources/DualModel.hpp"
#include "scene/Camera.hpp"
#include "scene/Transform.hpp"
#include "scene/SpiralScene.hpp"
#include "ui/ui.hpp"
#include "window.h"

int main()
{
	try
	{
		Window window(1820, 980, "SPIRAL BENCHMARK");
		Instance instance(false);
		auto surface = window.createSurface(instance);
		Device device(instance, *surface);
		Swapchain swapchain(device, *surface, window.getWidth(), window.getHeight());
		RenderPass renderPass(device, swapchain.getImageFormat());
		UniformBuffer uniformBuffer(device);
		Descriptor descriptor(device, uniformBuffer);


		SpiralPipeline pipeline(device, renderPass, swapchain.getExtent(), descriptor.getLayout());
		SpiralPipeline wireframePipeline(device, renderPass, swapchain.getExtent(), descriptor.getLayout(), vk::PolygonMode::eLine);

		FrameBuffer frameBuffer(device, renderPass, swapchain);

		CommandManager commandManager(device);
		commandManager.createCommandBuffers(static_cast<uint32_t>(swapchain.getImages().size()));

		std::string modelPath = "assets/Duck.gltf";

		SpiralScene spiralScene(device, commandManager, modelPath);
		UserInterface ui(instance, device, swapchain, renderPass, window, commandManager);

		Camera camera;
		camera.setPerspective(
			60.0f,
			static_cast<float>(swapchain.getExtent().width) / static_cast<float>(swapchain.getExtent().height),
			0.1f,
			50000.0f
		);

		camera.setView(
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 155.0f),
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		camera.setPosition(glm::vec3(0.0f, 0.0f, 155.0f));

		SpiralRenderer renderer(
			device,
			swapchain,
			renderPass,
			pipeline,
			frameBuffer,
			commandManager,
			window,
			surface.get(),
			spiralScene,
			uniformBuffer,
			descriptor
		);
		renderer.setWireframePipeline(wireframePipeline);

		bool cameraActive = false;
		window.setMouseCallback([&](double xPos, double yPos)
			{
				ui.handleMouseMove(xPos, yPos);

				if (cameraActive)
				{
					camera.handleMouseInput(xPos, yPos, cameraActive);
				}
			}
		);


		auto last = std::chrono::high_resolution_clock::now();
		while (!window.shouldClose())
		{
			auto current = std::chrono::high_resolution_clock::now();
			float delta = std::chrono::duration<float, std::chrono::seconds::period>(current - last).count();
			last = current;
			window.pollEvents();

			// end on escape
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
			spiralScene.updateSpiralPositions(delta);
			ui.beginFrame2(spiralScene, renderer);

			try
			{
				renderer.drawFrame(camera, ui);
			}
			catch (const vk::OutOfDateKHRError&)
			{
				renderer.recreateSwapchain();
			}
			catch (const std::exception& e)
			{
				std::cerr << "Error during frame rendering: " << e.what() << std::endl;
			}
			
			if (window.wasResized()) {
				window.resetResizedFlag();
				camera.setPerspective(
					60.0f,
					swapchain.getExtent().width / (float)swapchain.getExtent().height,
					0.1f,
					20000.0f
				);
			}
		}


		std::cout << "Completed base setup" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error during frame rendering: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}