/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file SimplificatorRenderer.cpp
 * @brief Implementation of the SimplificatorRenderer class for the Simplificator application.
 *
 * This file implements the SimplificatorRenderer class, which is responsible for
 * rendering the simplification scene in the Simplificator application.
 * 
 * Split-screen rendering is implemented to show the original model
 * on the left and the simplified model on the right. Wireframe mode
 * is also supported to visualize the mesh structure of both models.
 */

#include "SimplificatorRenderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <array>

SimplificatorRenderer::SimplificatorRenderer(Device& device, Swapchain& swapchain, RenderPass& renderPass,
    Pipeline& pipeline, FrameBuffer& framebuffer, CommandManager& commandManager, Window& window,
    vk::SurfaceKHR surface, Model& model, UniformBuffer& uniformBuffer, Descriptor& descriptor)
    : Renderer(device, swapchain, renderPass, framebuffer, commandManager, window, surface),
      m_pipeline(pipeline),
      m_model(&model),
      m_uniformBuffer(uniformBuffer),
      m_descriptor(descriptor)
{

}

void SimplificatorRenderer::drawSplitScreen(const Camera& camera, const Transform& transform, UserInterface& ui)
{
	// throw if dual model is not set, as its required for split-screen rendering
    if (!m_dualModel)
    {
        throw std::runtime_error("Dual model not set for split-screen rendering.");
    }

    // get next image from swapchain and begin command buffer recording
    uint32_t imgIdx;
    vk::CommandBuffer cmdBuffer = beginFrame(imgIdx);

    // handle window resize
    if (!cmdBuffer) return;

    std::array<vk::ClearValue, 2> clearValues = {};
	clearValues[0].color = vk::ClearColorValue(0.1f, 0.1f, 0.15f, 1.0f); // background color
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0); // clear depth to far plane

    vk::RenderPassBeginInfo renderPassInfo(
        m_renderPass.get(),
        m_framebuffer.getFrameBufferAt(imgIdx),
        vk::Rect2D({ 0,0 }, m_swapchain.getExtent()),
        clearValues.size(),
        clearValues.data()
    );

    cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    // bind pipeline
    if (showWireframe && m_wireframePipeline != nullptr)
    {
        cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_wireframePipeline->get());
    }
    else
    {
        cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());
    }

    Texture* sharedTexture = m_dualModel->getOriginalModel().getTexture();
    if (sharedTexture)
    {
        m_descriptor.updateTexture(currentFrame, *sharedTexture);
    }

    auto extent = m_swapchain.getExtent();
    float halfWidth = static_cast<float>(extent.width) / 2.0f;
    float aspectRatio = halfWidth / static_cast<float>(extent.height);

    // update uniform buffer (shared for both models)
    UniformBufferObject ubo{};
    ubo.model = transform.getModelMatrix();
    ubo.view = camera.getViewMatrix();
    ubo.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);
    ubo.cameraPos = camera.getPosition();
    ubo.proj[1][1] *= -1;

    m_uniformBuffer.update(ubo, currentFrame);

	vk::DescriptorSet descriptor = m_descriptor.get(currentFrame);
    cmdBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics,
        m_pipeline.getLayout(),
        0,
        1,
        &descriptor,
        0,
        nullptr
    );

    // draw left side (original model)
    setupViewportScissor(cmdBuffer, m_swapchain.getExtent(), m_swapchain.getExtent().width / 2, LEFT);
    m_dualModel->drawOriginalModel(cmdBuffer);

	// draw right side (simplified model)
    setupViewportScissor(cmdBuffer, m_swapchain.getExtent(), m_swapchain.getExtent().width / 2, RIGHT);
    m_dualModel->drawSimplifiedModel(cmdBuffer);

    // draw ui
    ui.render(cmdBuffer);
    cmdBuffer.endRenderPass();

    // present and sync
    endFrame(cmdBuffer, imgIdx);
}

void SimplificatorRenderer::setupViewportScissor(vk::CommandBuffer cmd, vk::Extent2D extent, uint32_t width, int side)
{
    vk::Viewport viewport;
    vk::Rect2D scissor;

    if (side == LEFT)
    {
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        scissor.offset = vk::Offset2D(0, 0);
        scissor.extent = vk::Extent2D(width, extent.height);
    }
    else if (side == RIGHT)
    {
        viewport.x = static_cast<float>(width);
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        scissor.offset = vk::Offset2D(width, 0);
        scissor.extent = vk::Extent2D(width, extent.height);
    }
    else
    {
        throw std::runtime_error("Invalid side for viewport and scissor setup.");
    }

    cmd.setViewport(0, 1, &viewport);
    cmd.setScissor(0, 1, &scissor);
}

/* End of the SimplificatorRenderer.cpp file */