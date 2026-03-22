/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file SimplificatorApp.hpp
 * @brief Header file for the Simplificator application.
 *
 * This file implements the SimplificatorApp class, which is a Vulkan application for mesh simplification techniques.
 * Sets up specific pipelines, scene and renderer for the Simplificator application.
 */

#pragma once
#include "../../common/core/VulkanApp.hpp"
#include "../../common/core/Pipeline.hpp"
#include "../../common/rendering/Renderer.hpp"
#include "../../common/resources/DualModel.hpp"
#include "../../common/scene/Transform.hpp"

class SimplificatorApp : public VulkanApp {
public:
	SimplificatorApp();

protected:
	void init() override;
	void update(float deltaTime) override;
	void drawFrame() override;

private:
	std::unique_ptr<Pipeline> pipeline;
	std::unique_ptr<Pipeline> wireframePipeline;
	std::unique_ptr<DualModel> dualModel;
	std::unique_ptr<Renderer> renderer;

	Transform transform;
	float xRotation = 0.0f;
	float yRotation = 0.0f;
	float zRotation = 0.0f;
};

/* End of the SimplificatorApp.hpp file */