/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file SpiralRenderer.hpp
 * @brief Implementation of the SpiralRenderer class for the Spiral application.
 *
 * This file implements the SpiralRenderer class, which is responsible for
 * rendering the spiral scene in the Spiral application.
 */

#pragma once

#include "common/rendering/Renderer.hpp"
#include "common/core/Pipeline.hpp"
#include "common/core/ComputePipeline.hpp"
#include "common/scene/Camera.hpp"
#include "apps/spiral/SpiralScene.hpp"
#include "common/rendering/UniformBuffer.hpp"
#include "common/rendering/Descriptors.hpp"
#include "common/window.h"

class UserInterface;
class SpiralComputePipeline;

class SpiralRenderer : public Renderer {
public:
    SpiralRenderer(
        Device& device,
        Swapchain& swapchain,
        RenderPass& renderPass,
        Pipeline& pipeline,
        FrameBuffer& framebuffer,
        CommandManager& commandManager,
        Window& window,
        vk::SurfaceKHR surface,
        SpiralScene& spiralScene,
        UniformBuffer& uniformBuffer,
        Descriptor& descriptor
    );

    ~SpiralRenderer() override = default;

    /**
    * @brief Records command buffers for spiral scene rendering.
    * 
	* @param camera The camera object containing view and projection matrices
	* @param ui The user interface object for rendering UI elements
    */
    void drawFrame(const Camera& camera, UserInterface& ui);

    /**
    * @brief Updates texture descriptors
    */
    void refreshTextureDescriptors();
    /**
    * @brief Updates compute descriptors
    */
	void refreshComputeDescriptors();

    // getters and setters
    void setWireframePipeline(Pipeline& pipeline) { m_wireframePipeline = &pipeline; }
    void setShowWireframe(bool show) { showWireframe = show; }
    void setComputePipeline(ComputePipeline& pipeline) { m_computePipeline = &pipeline; }
    void setUseGPULODCompute(bool use)
    {
        useGPULODCompute = use;
        if (!useGPULODCompute) useGPUSpiralCompute = false;
    }
    bool getUseGPULODCompute() const { return useGPULODCompute; }

    void setUseGPUSpiralCompute(bool use)
    {
        useGPUSpiralCompute = use;
        if (useGPUSpiralCompute) useGPULODCompute = true;
    }
    bool getUseGPUSpiralCompute() const { return useGPUSpiralCompute; }

private:
    Pipeline& m_pipeline;
    Pipeline* m_wireframePipeline = nullptr;
    ComputePipeline* m_computePipeline = nullptr;
    SpiralScene& m_spiralScene;
    UniformBuffer& m_uniformBuffer;
    Descriptor& m_descriptor;

    bool showWireframe = false;
    bool useGPULODCompute = false;
    bool useGPUSpiralCompute = false;
};

/* End of the SpiralRenderer.hpp file */