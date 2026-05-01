/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file SimplificatorRenderer.hpp
 * @brief Implementation of the SimplificatorRenderer class for the Simplificator application.
 *
 * This file implements the SimplificatorRenderer class, which is responsible for
 * rendering the simplification scene in the Simplificator application.
 * 
 * Split-screen rendering is implemented to show the original model
 * on the left and the simplified model on the right. Wireframe mode
 * is also supported to visualize the mesh structure of both models.
 */

#pragma once

#include "common/rendering/Renderer.hpp"
#include "common/core/Pipeline.hpp"
#include "common/resources/Model.hpp"
#include "common/scene/Camera.hpp"
#include "common/ui/ui.hpp"
#include "common/scene/Transform.hpp"
#include "common/resources/DualModel.hpp"
#include "common/rendering/UniformBuffer.hpp"
#include "common/rendering/Descriptors.hpp"
#include "common/core/Device.hpp"

class UserInterface;

class SimplificatorRenderer : public Renderer {
public:
    SimplificatorRenderer(
        Device& device,
        Swapchain& swapchain,
        RenderPass& renderPass,
        Pipeline& pipeline,
        FrameBuffer& framebuffer,
        CommandManager& commandManager,
        Window& window,
        vk::SurfaceKHR surface,
        Model& model,
        UniformBuffer& uniformBuffer,
        Descriptor& descriptor
    );

    ~SimplificatorRenderer() override = default;
    void drawSplitScreen(const Camera& camera, const Transform& transform, UserInterface& ui);

    void setModel(Model& newModel) { m_model = &newModel; }
    void setDualModel(DualModel& newDualModel) { m_dualModel = &newDualModel; }

    void setShowWireframe(bool show) { showWireframe = show; }
    void setWireframePipeline(Pipeline& pipeline) { m_wireframePipeline = &pipeline; }

private:
    void setupViewportScissor(vk::CommandBuffer cmd, vk::Extent2D extent, uint32_t width, int side);

    Pipeline& m_pipeline;
    Model* m_model;
    UniformBuffer& m_uniformBuffer;
    Descriptor& m_descriptor;
    DualModel* m_dualModel = nullptr;
    Pipeline* m_wireframePipeline = nullptr;

    bool showWireframe = false;
};

/* End of the SimplificatorRenderer.hpp file */