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
	: VulkanApp(1800, 920, false, "Spiral scene")
{

}

void SpiralApp::init()
{
    pipeline = std::make_unique<Pipeline>(device, renderPass, swapchain.getExtent(), descriptor.getLayout(), PipelineType::Spiral);
    wireframePipeline = std::make_unique<Pipeline>(device, renderPass, swapchain.getExtent(), descriptor.getLayout(), PipelineType::Spiral, vk::PolygonMode::eLine);
    spiralScene = std::make_unique<SpiralScene>(device, commandManager, "assets/sphere/sphere.gltf", uniformBuffer);
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
        if (benchmark.getMethod() == BenchmarkMethod::STATIC_CAMERA)
        {
            // apply benchmark config to the scene and renderer
            BenchmarkConfig cfg = benchmark.getCurrentConfig();
            spiralScene->config.instanceCount = cfg.instances;
            spiralScene->config.enableLOD = cfg.enableLOD;
            // ... leave the simplification params to the user via gui

            renderer->setUseGPULODCompute(cfg.useGPULOD);
            renderer->setUseGPUSpiralCompute(cfg.useGPUSpiral);

            // reset positions when changing instance count
            spiralScene->updateSpiralPositions(0.0f, false);

            benchmark.clearApplyConfigFlag();
        }
		else if (benchmark.getMethod() == BenchmarkMethod::MOVING_CAMERA)
		{
            // setup the scene
            spiralScene->config.coneFactor = 10.0f;
            spiralScene->config.instanceCount = 7008; // must be divisible by 12 to prevent artifacts
            spiralScene->config.speed = 1.0f;
            spiralScene->config.numArms = 12;
            spiralScene->config.twistSpeed = 0.2f;
            spiralScene->config.spacing = 0.1f;
            spiralScene->config.lodDist0 = 1000.0f;
            spiralScene->config.lodDist1 = 2000.0f;
            spiralScene->config.lodDist2 = 4000.0f;
            
			// reset camera to the start position
            camera.setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            camera.setPerspective(60.0f, swapchain.getExtent().width / (float)swapchain.getExtent().height, 0.1f, 100000.0f);
            spiralScene->updateSpiralPositions(deltaTime, false);

			// reset benchmark timer and data
            benchmark.clearApplyConfigFlag();
		}
    }

	// update spiral positions and LODs
    // will exit early if GPU mode is on
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

	// adjust far plane and perspective based on instance count to avoid clipping
    float farPlaneChange = (spiralScene->config.instanceCount * spiralScene->config.spacing) + 5000.0f;
    if (window.wasResized() || farPlaneChange > farPlane)
    {
        window.resetResizedFlag();
		farPlane = farPlaneChange;

        float width = std::max(1.0f, static_cast<float>(window.getWidth()));
        float height = std::max(1.0f, static_cast<float>(window.getHeight()));

        camera.setPerspective(60.0f, width / height, 0.1f, farPlane);
    }

    if (benchmark.isRunning())
    {
        glm::vec3 currentCamPos = camera.getPosition();
        benchmark.update(deltaTime, *spiralScene, *renderer, currentCamPos);
        camera.setPosition(currentCamPos);
    }

	// check if benchmark just ended, if yes, reset the scene to default values
    if (wasBenchmarkRunning && !benchmark.isRunning())
    {
        spiralScene->config.instanceCount = 10000;
        spiralScene->config.enableLOD = true;
		renderer->setUseGPULODCompute(false);
		renderer->setUseGPUSpiralCompute(false);
		spiralScene->updateSpiralPositions(0.0f, false);
        spiralScene->config.coneFactor = 0.5f;
        spiralScene->config.speed = 30.0f;
        spiralScene->config.numArms = 5;
        spiralScene->config.twistSpeed = 0.02f;
        spiralScene->config.spacing = 2.0f;
        spiralScene->config.lodDist0 = 400.0f;
        spiralScene->config.lodDist1 = 1200.0f;
        spiralScene->config.lodDist2 = 3000.0f;
        camera.setPosition(glm::vec3(0.0f, 0.0f, 155.0f));
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