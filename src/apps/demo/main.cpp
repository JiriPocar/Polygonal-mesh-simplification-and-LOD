#include <vulkan/vulkan.hpp>
#include <iostream>
#include <chrono>
#include <vector>
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
#include "resources/DualModel.hpp"
#include "scene/Camera.hpp"
#include "scene/Transform.hpp"
#include "ui/ui.hpp"
#include "window.h"

int main()
{
	try
	{
		Window window(1400, 800, "TIME SPIRAL");
		Instance instance(false);
		auto surface = window.createSurface(instance);
		Device device(instance, *surface);
		Swapchain swapchain(device, *surface, window.getWidth(), window.getHeight());
		RenderPass renderPass(device, swapchain.getImageFormat());
		UniformBuffer uniformBuffer(device);
		Descriptor descriptor(device, uniformBuffer);
		Pipeline pipeline(device, renderPass, swapchain.getExtent(), descriptor.getLayout());
		FrameBuffer framebuffer(device, renderPass, swapchain);
		CommandManager commandManager(device);
		commandManager.createCommandBuffers(static_cast<uint32_t>(swapchain.getImages().size()));
		UserInterface ui(instance, device, swapchain, renderPass, window, commandManager);

		std::unique_ptr<DualModel> model = std::make_unique<DualModel>(device, "assets/Fox.gltf");

		Camera cam;
		cam.setPerspective(45.0f, swapchain.getExtent().width / (float)swapchain.getExtent().height, 0.1f, 2000.0f);
		cam.setView(glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		Transform transform;
		float baseScale = model->getOriginalModel().getScaleIndex();

		Renderer renderer(device, swapchain, renderPass, pipeline, framebuffer,
			commandManager, window, surface.get(), model->getOriginalModel(), uniformBuffer, descriptor);

		renderer.setDualModel(*model);

        /*while (!window.shouldClose())
        {
            ui.beginFrame(model, device, renderer, transform);

			try {
				renderer.drawFrame(cam, transform, ui);
			}
			catch (const vk::OutOfDateKHRError&) {
				renderer.recreateSwapchain();
			}
			catch (const std::exception& e) {
				std::cerr << "Error during frame rendering: " << e.what() << std::endl;
			}

            if (window.wasResized()) {
                window.resetResizedFlag();
                cam.setPerspective(45.0f, swapchain.getExtent().width / (float)swapchain.getExtent().height, 0.1f, 2000.0f);
            }
        }*/

		std::cout << "Completed base setup" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error during frame rendering: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}