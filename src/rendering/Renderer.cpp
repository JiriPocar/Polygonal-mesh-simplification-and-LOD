#include "Renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <iostream>

Renderer::Renderer(Device& device, Swapchain& swapchain, RenderPass& renderPass,
	Pipeline& pipeline, FrameBuffer& framebuffer, CommandManager& commandManager,
	Window& window, vk::SurfaceKHR surface, Model& model, UniformBuffer& uniformBuffer, Descriptor& descriptor)
	:	m_device(device),
		m_swapchain(swapchain),
		m_renderPass(renderPass),
		m_pipeline(pipeline),
		m_framebuffer(framebuffer),
		m_commandManager(commandManager),
		m_window(window),
		m_surface(surface),
		m_model(model),
		m_uniformBuffer(uniformBuffer),
		m_descriptor(descriptor)
{
	createSyncObjects();

	m_window.setResizeCallback([this](int width, int height)
		{
			this->framebufferResized = true;
		});
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

void Renderer::drawFrame(const Camera& camera, const Transform& transform, UserInterface& ui)
{
	if (framebufferResized)
	{
		recreateSwapchain();
		return;
	}

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
	
	UniformBufferObject ubo{};
	ubo.model = transform.getModelMatrix();
	ubo.view = camera.getViewMatrix();
	ubo.proj = camera.getProjectionMatrix();

	m_uniformBuffer.update(ubo);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
								 m_pipeline.getLayout(),
								 0,
								 1,
								 &m_descriptor.get(),
								 0,
								 nullptr);

	m_model.draw(cmdBuffer);

	ui.render(cmdBuffer);

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

void Renderer::recreateSwapchain()
{
	int width = 0, height = 0;

	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_window.getGLFWWindow(), &width, &height);
		glfwWaitEvents();
	}

	m_device.operator*().waitIdle();

	m_swapchain.recreateOnResize(m_surface, width, height);

	recreateFramebuffers();

	framebufferResized = false;
}

void Renderer::recreateFramebuffers()
{
	m_framebuffer.cleanup();
	m_framebuffer.createFramebuffers(m_renderPass, m_swapchain);
}