#include "Renderer.hpp"
#include <stdexcept>
#include <iostream>

Renderer::Renderer(Device& device, Swapchain& swapchain, RenderPass& renderPass,
	Pipeline& pipeline, FrameBuffer& framebuffer, CommandManager& commandManager)
	:	m_device(device),
		m_swapchain(swapchain),
		m_renderPass(renderPass),
		m_pipeline(pipeline),
		m_framebuffer(framebuffer),
		m_commandManager(commandManager)
{
	createSyncObjects();
}

Renderer::~Renderer()
{
	cleanupSyncObjects();
}

void Renderer::createSyncObjects()
{
	vk::SemaphoreCreateInfo semaphoreInfo;
	vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);

	imageAvailableSemaphores = m_device.operator*().createSemaphoreUnique(semaphoreInfo);
	renderFinishedSemaphores = m_device.operator*().createSemaphoreUnique(semaphoreInfo);
	inFlightFence = m_device.operator*().createFenceUnique(fenceInfo);

	if (!imageAvailableSemaphores || !renderFinishedSemaphores || !inFlightFence) {
		throw std::runtime_error("Failed to create synchronization objects for a frame!");
	}
}

void Renderer::cleanupSyncObjects()
{
	m_device.operator*().waitIdle();
}

void Renderer::drawFrame()
{
	// wait for gpu to finish frame
	m_device.operator*().waitForFences(1, &*inFlightFence, VK_TRUE, UINT64_MAX);
	m_device.operator*().resetFences(1, &*inFlightFence);

	// get next image from swapchain
	uint32_t imgIdx = m_swapchain.acquireNextImage(*imageAvailableSemaphores);

	// record command buffer
	vk::CommandBuffer cmdBuffer = m_commandManager.getCommandBuffer(imgIdx);
	cmdBuffer.reset();

	vk::CommandBufferBeginInfo info;
	cmdBuffer.begin(info);

	// render pass
	vk::ClearValue clearColor(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f));

	vk::RenderPassBeginInfo renderPassInfo(
		m_renderPass.get(),
		m_framebuffer.getFrameBufferAt(imgIdx),
		vk::Rect2D({0,0}, m_swapchain.getExtent()),
		1,
		&clearColor
	);

	cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	// bind pipeline a draw call
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());
	cmdBuffer.draw(3, 1, 0, 0);

	cmdBuffer.endRenderPass();
	cmdBuffer.end();

	// submit command buffer
	vk::PipelineStageFlags pipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);

	vk::SubmitInfo submitInfo(
		1,
		&*imageAvailableSemaphores,
		&pipelineStageFlags,
		1,
		&cmdBuffer,
		1,
		&*renderFinishedSemaphores
	);

	m_device.getGraphicsQueue().submit(1, &submitInfo, *inFlightFence);

	// present image-
	vk::Semaphore waitSemaphore = *renderFinishedSemaphores;
	vk::SwapchainKHR rawSwapchain = m_swapchain.get().get();

	vk::PresentInfoKHR presentInfo(
		1,                      // waitSemaphoreCount
		&waitSemaphore,         // pWaitSemaphores
		1,                      // swapchainCount
		&rawSwapchain,             // pSwapchains
		&imgIdx,            // pImageIndices
		nullptr                 // pResults
	);

	m_device.getPresentQueue().presentKHR(presentInfo);
}