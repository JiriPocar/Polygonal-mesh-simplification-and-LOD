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
	: VulkanApp(1800, 920, "Spiral scene")
{

}

void SpiralApp::init()
{
    pipeline = std::make_unique<Pipeline>(device, renderPass, swapchain.getExtent(), descriptor.getLayout(), PipelineType::Spiral);
    wireframePipeline = std::make_unique<Pipeline>(device, renderPass, swapchain.getExtent(), descriptor.getLayout(), PipelineType::Spiral, vk::PolygonMode::eLine);
    spiralScene = std::make_unique<SpiralScene>(device, commandManager, "assets/sphere.gltf", uniformBuffer);
    descriptor.createComputeDescriptors(*spiralScene);
    computePipeline = std::make_unique<ComputePipeline>(device, descriptor.getComputeLayout(), sizeof(ComputePushConstants));

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
    bool wasBenchmarkRunning = benchmark.isRunning();

    if (benchmark.isRunning() && benchmark.needsApplyConfig())
    {
		// apply benchmark config to the scene and renderer
		BenchmarkConfig cfg = benchmark.getCurrentConfig();
		spiralScene->config.instanceCount = cfg.instances;
		spiralScene->config.enableLOD = cfg.enableLOD;
		renderer->setUseGPULODCompute(cfg.useGPULOD);
		renderer->setUseGPUSpiralCompute(cfg.useGPUSpiral);

        // reset positions when changing instance count
		spiralScene->updateSpiralPositions(0.0f, false);

		benchmark.clearApplyConfigFlag();
    }

	// update spiral positions and LODs
    spiralScene->updateSpiralPositions(deltaTime, renderer->getUseGPUSpiralCompute());
    spiralScene->updateLODs(camera.getPosition(), renderer->getCurrentFrame(), renderer->getUseGPULODCompute(), renderer->getUseGPUSpiralCompute());
    
    if (benchmark.isRunning())
    {
        // disable main ui during benchmark
        ui.beginFrame2(*spiralScene, *renderer, camera.getPosition(), benchmark, false);
    }
    else
    {
        ui.beginFrame2(*spiralScene, *renderer, camera.getPosition(), benchmark, showUI);
    }

    // handle window resizing
    if (window.wasResized())
    {
        window.resetResizedFlag();
        camera.setPerspective(60.0f, swapchain.getExtent().width / (float)swapchain.getExtent().height, 0.1f, 20000.0f);
    }

    if (benchmark.isRunning())
    {
		benchmark.update(deltaTime, *spiralScene, camera.getPosition());
    }

	// check if benchmark just ended, if yes, reset the scene to default values
    if (wasBenchmarkRunning && !benchmark.isRunning())
    {
        spiralScene->config.instanceCount = 10000;
        spiralScene->config.enableLOD = true;
		renderer->setUseGPULODCompute(false);
		renderer->setUseGPUSpiralCompute(false);
		spiralScene->updateSpiralPositions(0.0f, false);
    }
}

void SpiralApp::drawFrame()
{
    renderer->drawFrame(camera, ui);
}

/* End of the SpiralApp.cpp file */