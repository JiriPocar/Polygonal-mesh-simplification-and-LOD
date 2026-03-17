/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file SpiralApp.cpp
 * @brief Source file for the Spiral application.
 *
 * This file implements the SpiralApp class, which is a Vulkan application for
 * rendering a mathematical spiral with various features such as LOD, GPU-driven
 * rendering, and performance benchmarking.
 *
 * Sets up specific pipelines, scene and renderer for the Spiral application.
 */

#include "SpiralApp.hpp"
#include <iostream>

SpiralApp::SpiralApp() 
	: VulkanApp(1800, 900, "Spiral scene")
{

}

void SpiralApp::init()
{
    pipeline = std::make_unique<SpiralPipeline>(device, renderPass, swapchain.getExtent(), descriptor.getLayout());
    wireframePipeline = std::make_unique<SpiralPipeline>(device, renderPass, swapchain.getExtent(), descriptor.getLayout(), vk::PolygonMode::eLine);
    spiralScene = std::make_unique<SpiralScene>(device, commandManager, "assets/Duck.gltf", uniformBuffer);
    descriptor.createComputeDescriptors(*spiralScene);
    computePipeline = std::make_unique<SpiralComputePipeline>(device, descriptor.getComputeLayout());

    camera.setPerspective(60.0f, static_cast<float>(swapchain.getExtent().width) / static_cast<float>(swapchain.getExtent().height), 0.1f, 50000.0f);
    camera.setView(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 155.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    camera.setPosition(glm::vec3(0.0f, 0.0f, 155.0f));

    renderer = std::make_unique<SpiralRenderer>(
        device,
        swapchain,
        renderPass,
        *pipeline,
        framebuffer,
        commandManager,
        window,
        surface.get(),
        *spiralScene,
        uniformBuffer,
        descriptor
    );
    renderer->setWireframePipeline(*wireframePipeline);
    renderer->setComputePipeline(*computePipeline);
}

void SpiralApp::update(float deltaTime)
{
    spiralScene->updateSpiralPositions(deltaTime, renderer->getUseGPUSpiralCompute());
    ui.beginFrame2(*spiralScene, *renderer, showUI);

    // handle window resizing
    if (window.wasResized())
    {
        window.resetResizedFlag();
        camera.setPerspective(60.0f, swapchain.getExtent().width / (float)swapchain.getExtent().height, 0.1f, 20000.0f);
    }
}

void SpiralApp::drawFrame()
{
    try {
        renderer->drawFrame(camera, ui);
    }
    catch (const vk::OutOfDateKHRError&) {
        renderer->recreateSwapchain();
    }
    catch (const std::exception& e) {
        std::cerr << "Error during frame rendering: " << e.what() << std::endl;
    }
}

/* End of the SpiralApp.cpp file */