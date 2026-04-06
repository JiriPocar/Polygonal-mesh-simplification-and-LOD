/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file SpiralApp.hpp
 * @brief Header file for the Simplificator application.
 *
 * This file implements the SpiralApp class, which is a Vulkan application for
 * rendering a mathematical spiral with various features such as LOD, GPU-driven
 * rendering, and performance benchmarking.
 * 
 * Sets up specific pipelines, scene and renderer for the Spiral application.
 */

#pragma once
#include "../../common/core/VulkanApp.hpp"
#include "../../common/core/Pipeline.hpp"
#include "../../common/core/ComputePipeline.hpp"
#include "SpiralScene.hpp"
#include "SpiralRenderer.hpp"
#include "Benchmark.hpp"

class SpiralApp : public VulkanApp {
public:
	SpiralApp();

protected:
	void init() override;
	void update(float deltaTime) override;
	void drawFrame() override;

private:
	std::unique_ptr<Pipeline> pipeline;
	std::unique_ptr<Pipeline> wireframePipeline;
	std::unique_ptr<ComputePipeline> computePipeline;
	std::unique_ptr<SpiralScene> spiralScene;
	std::unique_ptr<SpiralRenderer> renderer;

	Benchmark benchmark;
};

/* End of the SpiralApp.hpp file */