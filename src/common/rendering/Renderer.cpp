#include "Renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <iostream>
#include <array>
#include "../ui/ui.hpp"

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
		m_model(&model),
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

	m_uniformBuffer.update(ubo, 1);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
								 m_pipeline.getLayout(),
								 0,
								 1,
								 &m_descriptor.get(1),
								 0,
								 nullptr);

	m_model->draw(cmdBuffer);

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

void Renderer::drawSplitScreen(const Camera& camera, const Transform& transform, UserInterface& ui)
{
	if (!m_dualModel)
	{
		throw std::runtime_error("Dual model not set for split-screen rendering.");
	}

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
	std::array<vk::ClearValue, 2> clearValues = {};
	clearValues[0].color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f); // background color
	clearValues[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0); // depth clear

	vk::RenderPassBeginInfo renderPassInfo(
		m_renderPass.get(),
		m_framebuffer.getFrameBufferAt(imgIdx),
		vk::Rect2D({ 0,0 }, m_swapchain.getExtent()),
		clearValues.size(),
		clearValues.data()
	);

	cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	// decide which pipeline to use based on wireframe mode
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

	UniformBufferObject ubo{};
	ubo.model = transform.getModelMatrix();
	ubo.view = camera.getViewMatrix();
	ubo.proj = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 1000.0f);
	ubo.proj[1][1] *= -1;

	m_uniformBuffer.update(ubo, currentFrame);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		m_pipeline.getLayout(),
		0,
		1,
		&m_descriptor.get(currentFrame),
		0,
		nullptr
	);

	// draw left side (original model)
	setupViewportScissor(cmdBuffer, m_swapchain.getExtent(), m_swapchain.getExtent().width / 2, LEFT);
	m_dualModel->drawOriginalModel(cmdBuffer);

	// draw right side (simplified model)
	setupViewportScissor(cmdBuffer, m_swapchain.getExtent(), m_swapchain.getExtent().width / 2, RIGHT);
	m_dualModel->drawSimplifiedModel(cmdBuffer);

	// draw UI
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

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::setupViewportScissor(vk::CommandBuffer cmd, vk::Extent2D extent, uint32_t width, int side)
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