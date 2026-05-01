/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file SpiralRenderer.cpp
 * @brief Implementation of the SpiralRenderer class for the Spiral application.
 *
 * This file implements the SpiralRenderer class, which is responsible for
 * rendering the spiral scene in the Spiral application.
 * 
 * Rendering the spiral scene with support for GPU-based LOD computation and
 * optional wireframe mode. The renderer updates the spiral scene's LODs each
 * frame based on the camera position,
 */

#include "SpiralRenderer.hpp"
#include "common/ui/ui.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <array>

SpiralRenderer::SpiralRenderer(Device& device,
    Swapchain& swapchain, RenderPass& renderPass,
    Pipeline& pipeline, FrameBuffer& framebuffer,
    CommandManager& commandManager, Window& window,
    vk::SurfaceKHR surface, SpiralScene& spiralScene,
    UniformBuffer& uniformBuffer, Descriptor& descriptor)
    :   Renderer(device, swapchain, renderPass, framebuffer, commandManager, window, surface),
        m_pipeline(pipeline),
        m_spiralScene(spiralScene),
        m_uniformBuffer(uniformBuffer),
        m_descriptor(descriptor)
{
	refreshTextureDescriptors();
}

void SpiralRenderer::drawFrame(const Camera& camera, UserInterface& ui)
{
	// get next image from swapchain and begin command buffer recording
    uint32_t imgIdx;
    vk::CommandBuffer cmdBuffer = beginFrame(imgIdx);

    // handle window resize
    if (!cmdBuffer) return;

	// update uniform buffer
    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    ubo.view = camera.getViewMatrix();
    ubo.proj = camera.getProjectionMatrix();
    ubo.cameraPos = camera.getPosition();
    ubo.showLodColors = showWireframe ? 1 : 0;
    m_uniformBuffer.update(ubo, currentFrame);

    if (useGPULODCompute)
    {
        // reset indirect buffer before dispatching compute shader to ensure its empty
        m_spiralScene.resetIndirectBuffer(cmdBuffer, currentFrame);

        // barrier to ensure indirect buffer is ready for compute shader to write after reset
        vk::BufferMemoryBarrier updateBarrier(
            vk::AccessFlagBits::eTransferWrite,
            vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            m_spiralScene.getIndirectBuffer(currentFrame),
            0,
            VK_WHOLE_SIZE
        );

        cmdBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eComputeShader,
            {},
            0,
            nullptr,
            1,
            &updateBarrier,
            0,
            nullptr
        );

		// bind compute pipeline and descriptor sets
        cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_computePipeline->get());
        cmdBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eCompute,
            m_computePipeline->getLayout(),
            0,
            1,
            &m_descriptor.getCompute(currentFrame),
            0,
            nullptr
        );

		// gather push constants for compute shader
        ComputePushConstants pcs = {};
        pcs.totalInstances = m_spiralScene.config.instanceCount;
        pcs.lodDist0 = m_spiralScene.config.lodDist0 * m_spiralScene.config.lodDist0;
        pcs.lodDist1 = m_spiralScene.config.lodDist1 * m_spiralScene.config.lodDist1;
        pcs.lodDist2 = m_spiralScene.config.lodDist2 * m_spiralScene.config.lodDist2;
        pcs.computeSpiral = useGPUSpiralCompute ? 1 : 0;
        pcs.enableLOD = m_spiralScene.config.enableLOD ? 1 : 0;
        pcs.spacing = m_spiralScene.config.spacing;
        pcs.numArms = m_spiralScene.config.numArms;
        pcs.minRadius = m_spiralScene.config.minRadius;
        pcs.coneFactor = m_spiralScene.config.coneFactor;
        pcs.twistSpeed = m_spiralScene.config.twistSpeed;
        pcs.animationTime = m_spiralScene.getAnimationTime();

        cmdBuffer.pushConstants(
            m_computePipeline->getLayout(),
            vk::ShaderStageFlagBits::eCompute,
            0,
            sizeof(ComputePushConstants),
            &pcs
        );

        uint32_t groupCount = (m_spiralScene.config.instanceCount + 255) / 256;

        // dispatch group count for compute shader to update instance data and indirect draw commands
        cmdBuffer.dispatch(groupCount, 1, 1);

        std::array<vk::BufferMemoryBarrier, 2> barriers = {
            // wait for compute shader to finish writing indirect draw commands before allowing indirect draw to read them
            vk::BufferMemoryBarrier(
                vk::AccessFlagBits::eShaderWrite,
                vk::AccessFlagBits::eIndirectCommandRead,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                m_spiralScene.getIndirectBuffer(currentFrame),
                0,
                VK_WHOLE_SIZE
            ),
            // wait for compute shader to finish writing instance data before allowing vertex shader to read them
            vk::BufferMemoryBarrier(
                vk::AccessFlagBits::eShaderWrite,
                vk::AccessFlagBits::eVertexAttributeRead,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                m_spiralScene.getLODInstanceBuffer(currentFrame),
                0,
                VK_WHOLE_SIZE
            )
        };

        cmdBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eDrawIndirect | vk::PipelineStageFlagBits::eVertexInput,
            {},
            0,
            nullptr,
            barriers.size(),
            barriers.data(),
            0,
            nullptr
        );
    }

    std::array<vk::ClearValue, 2> clearValues = {};
    clearValues[0].color = vk::ClearColorValue(0.1f, 0.1f, 0.15f, 1.0f); // background color
    clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0); // clear depth to far plane

    vk::RenderPassBeginInfo renderPassInfo(
        m_renderPass.get(),
        m_framebuffer.getFrameBufferAt(imgIdx),
        vk::Rect2D({ 0, 0 }, m_swapchain.getExtent()),
        clearValues.size(),
        clearValues.data()
    );

    cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    // bind pipeline
    if (showWireframe && m_wireframePipeline != nullptr)
    {
        cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_wireframePipeline->get());

        // bind descriptor sets
        cmdBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            m_wireframePipeline->getLayout(),
            0,
            1,
            &m_descriptor.get(currentFrame),
            0,
            nullptr
        );
    }
    else
    {
        cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

        // bind descriptor sets
        cmdBuffer.bindDescriptorSets(
            vk::PipelineBindPoint::eGraphics,
            m_pipeline.getLayout(),
            0,
            1,
            &m_descriptor.get(currentFrame),
            0,
            nullptr
        );
    }

    vk::Viewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapchain.getExtent().width);
    viewport.height = static_cast<float>(m_swapchain.getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    cmdBuffer.setViewport(0, 1, &viewport);

    vk::Rect2D scissor{};
    scissor.offset = vk::Offset2D{ 0, 0 };
    scissor.extent = m_swapchain.getExtent();
    cmdBuffer.setScissor(0, 1, &scissor);

    vk::Buffer instanceBuffer[] = { useGPULODCompute ? m_spiralScene.getLODInstanceBuffer(currentFrame) : m_spiralScene.getInstanceBuffer(currentFrame) };
    vk::DeviceSize instanceOffsets[] = { 0 };
    cmdBuffer.bindVertexBuffers(1, 1, instanceBuffer, instanceOffsets);

    for (int lod = 0; lod < 4; lod++)
    {
        // get model for this LOD level
        Model& lodModel = m_spiralScene.getModelLODSet().getLOD(lod);

        // smaller vertex buffer for each LOD level
        vk::Buffer vertexBuffers[] = { lodModel.getVertexBuffer() };
        vk::DeviceSize offsets[] = { 0 };
        cmdBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);
        cmdBuffer.bindIndexBuffer(lodModel.getIndexBuffer(), 0, vk::IndexType::eUint32);

        if (useGPULODCompute) // GPU computed, drawing with indirect commands
        {
            cmdBuffer.drawIndexedIndirect(
                m_spiralScene.getIndirectBuffer(currentFrame),	// buffer with draw commands generated by compute shader
                lod * sizeof(DrawIndexedIndirectCommand),		// offset to the command for this LOD level 
                1,												// draw one command for this LOD level
                sizeof(DrawIndexedIndirectCommand)				// stride between commands
            );
        }
        else // CPU computed, drawing traditionally
        {
            uint32_t instanceCount = m_spiralScene.getLODCount(lod);
            uint32_t firstInstance = m_spiralScene.getLODOffset(lod);

            cmdBuffer.drawIndexed(
                lodModel.getIndexCount(),
                instanceCount,
                0,
                0,
                firstInstance
            );
        }
    }

    ui.render(cmdBuffer);
    cmdBuffer.endRenderPass();

	// present and sync
    endFrame(cmdBuffer, imgIdx);
}

void SpiralRenderer::refreshTextureDescriptors()
{
    Texture* texture = m_spiralScene.getModelLODSet().getLOD(0).getTexture();
    if (texture)
    {
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            m_descriptor.updateTexture(i, *texture);
        }
    }
}

void SpiralRenderer::refreshComputeDescriptors()
{
    m_device.operator*().waitIdle();
	m_descriptor.updateDescriptorsCompute(m_spiralScene);
}

/* End of the SpiralRenderer.cpp file */