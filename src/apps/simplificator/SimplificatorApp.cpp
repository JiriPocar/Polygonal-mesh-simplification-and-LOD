/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file SimplificatorApp.cpp
 * @brief Source file for the Simplificator application.
 *
 * This file implements the SimplificatorApp class, which is a Vulkan application
 * that demonstrates a mesh simplification techniques.
 */

#include "SimplificatorApp.hpp"
#include <iostream>

SimplificatorApp::SimplificatorApp()
    : VulkanApp(1800, 900, "Simplificator")
{

}

void SimplificatorApp::init()
{
    pipeline = std::make_unique<Pipeline>(device, renderPass, swapchain.getExtent(), descriptor.getLayout());
    wireframePipeline = std::make_unique<Pipeline>(device, renderPass, swapchain.getExtent(), descriptor.getLayout(), vk::PolygonMode::eLine);
    dualModel = std::make_unique<DualModel>(device, commandManager, "assets/Duck.gltf");

    transform.setPos(glm::vec3(0.0f, 0.0f, 0.0f));
    transform.setRot(glm::vec3(0.0f, 0.0f, 0.0f));
    float modelScale = dualModel->getOriginalModel().getScaleIndex();
    transform.setScale(glm::vec3(modelScale, modelScale, modelScale));

    renderer = std::make_unique<SimplificatorRenderer>(
        device,
        swapchain,
        renderPass,
        *pipeline,
        framebuffer,
        commandManager,
        window,
        surface.get(),
        dualModel->getOriginalModel(),
        uniformBuffer,
        descriptor
    );
    renderer->setDualModel(*dualModel);
    renderer->setWireframePipeline(*wireframePipeline);
}

void SimplificatorApp::update(float deltaTime)
{
    Transform t = ui.fetchTransform();
    xRotation = t.getRot().x;
    yRotation = t.getRot().y;
    zRotation = t.getRot().z;

    auto rotationAxes = ui.getRotationAxes();
    bool rotate = (rotationAxes[0] || rotationAxes[1] || rotationAxes[2]);

    if (rotate)
    {
        if (rotationAxes[0])
        {
            xRotation += 50.0f * deltaTime;
            if (xRotation >= 360.0f) xRotation -= 360.0f;
        }

        if (rotationAxes[1])
        {
            yRotation += 50.0f * deltaTime;
            if (yRotation >= 360.0f) yRotation -= 360.0f;
        }

        if (rotationAxes[2])
        {
            zRotation += 50.0f * deltaTime;
            if (zRotation >= 360.0f) zRotation -= 360.0f;
        }
    }

    transform.setRot(glm::vec3(xRotation, yRotation, zRotation));
    ui.setTransform(transform);

    ui.beginFrame(dualModel, device, *renderer, transform, showUI);

	// handle window resizing
    if (window.wasResized())
    {
        window.resetResizedFlag();
        camera.setPerspective(45.0f, swapchain.getExtent().width / (float)swapchain.getExtent().height, 0.1f, 1000.0f);
    }
}

void SimplificatorApp::drawFrame()
{
	try {
		renderer->drawSplitScreen(camera, transform, ui);
	}
	catch (const vk::OutOfDateKHRError&) {
		renderer->recreateSwapchain();
	}
	catch (const std::exception& e) {
		std::cerr << "Error during frame rendering: " << e.what() << std::endl;
	}
}

/* End of the SimplificatorApp.cpp file */