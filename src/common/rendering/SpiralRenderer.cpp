#include "SpiralRenderer.hpp"
#include "../ui/ui.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <stdexcept>
#include <iostream>
#include <unordered_map>

SpiralRenderer::SpiralRenderer(
	Device& device,
	Swapchain& swapchain,
	RenderPass& renderPass,
	SpiralPipeline& pipeline,
	FrameBuffer& framebuffer,
	CommandManager& commandManager,
	Window& window,
	vk::SurfaceKHR surface,
	SpiralScene& spiralScene,
	UniformBuffer& uniformBuffer,
	Descriptor& descriptor
) :
	m_device(device),
	m_swapchain(swapchain),
	m_renderPass(renderPass),
	m_pipeline(pipeline),
	m_framebuffer(framebuffer),
	m_commandManager(commandManager),
	m_window(window),
	m_surface(surface),
	m_spiralScene(spiralScene),
	m_uniformBuffer(uniformBuffer),
	m_descriptor(descriptor)
{
	createSyncObjects();

	m_window.setResizeCallback([this](int width, int height)
		{
			this->framebufferResized = true;
		});
}

SpiralRenderer::~SpiralRenderer()
{
	cleanupSyncObjects();
}

void SpiralRenderer::createSyncObjects()
{
	vk::SemaphoreCreateInfo semaphoreInfo;
	vk::FenceCreateInfo fenceInfo(vk::FenceCreateFlagBits::eSignaled);

	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFence.resize(MAX_FRAMES_IN_FLIGHT);

	size_t imgCount = m_swapchain.getImages().size();
	imagesInFlight.resize(imgCount, VK_NULL_HANDLE);

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		imageAvailableSemaphores[i] = m_device.operator*().createSemaphoreUnique(semaphoreInfo);
		renderFinishedSemaphores[i] = m_device.operator*().createSemaphoreUnique(semaphoreInfo);
		inFlightFence[i] = m_device.operator*().createFenceUnique(fenceInfo);

		if (!imageAvailableSemaphores[i] || !renderFinishedSemaphores[i] || !inFlightFence[i])
		{
			throw std::runtime_error("Failed to create synchronization objects for a frame!");
		}
	}
}

void SpiralRenderer::cleanupSyncObjects()
{
	m_device.operator*().waitIdle();
}

void SpiralRenderer::drawFrame(const Camera& camera, UserInterface& ui)
{
	if (framebufferResized)
	{
		recreateSwapchain();
		return;
	}
	
	// wait for gpu to finish frame
	m_device.operator*().waitForFences(
		1,
		&*inFlightFence[currentFrame],
		VK_TRUE,
		UINT64_MAX
	);

	// update lods
	m_spiralScene.updateLODs(camera.getPosition());

	// get next image from swapchain
	uint32_t imgIdx = m_swapchain.acquireNextImage(*imageAvailableSemaphores[currentFrame]);

	// wait if image is already in use
	if (imagesInFlight[imgIdx] != VK_NULL_HANDLE)
	{
		m_device.operator*().waitForFences(
			1,
			&imagesInFlight[imgIdx],
			VK_TRUE,
			UINT64_MAX
		);
	}

	// mark image as now being in use by this frame
	imagesInFlight[imgIdx] = *inFlightFence[currentFrame];

	// reset fence for current frame
	m_device.operator*().resetFences(1, &*inFlightFence[currentFrame]);

	// record command buffer
	vk::CommandBuffer cmdBuffer = m_commandManager.getCommandBuffer(imgIdx);
	cmdBuffer.reset();

	vk::CommandBufferBeginInfo beginInfo;
	cmdBuffer.begin(beginInfo);

	// update uniform buffer
	UniformBufferObject ubo{};
	ubo.model = glm::mat4(1.0f); // wont be used
	ubo.view = camera.getViewMatrix();
	ubo.proj = camera.getProjectionMatrix();
	m_uniformBuffer.update(ubo);

	// render pass
	vk::ClearValue clearColor(vk::ClearColorValue(0.1f, 0.1f, 0.15f, 1.0f));
	vk::RenderPassBeginInfo renderPassInfo(
		m_renderPass.get(),
		m_framebuffer.getFrameBufferAt(imgIdx),
		vk::Rect2D({ 0, 0 }, m_swapchain.getExtent()),
		1,
		&clearColor
	);

	cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	// bind pipeline
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

	// bind descriptor sets
	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
		m_pipeline.getLayout(),
		0,
		1,
		&m_descriptor.get(),
		0,
		nullptr);

	vk::Buffer instanceBuffer[] = { m_spiralScene.getInstanceBuffer() };
	vk::DeviceSize instanceOffsets[] = { 0 };
	cmdBuffer.bindVertexBuffers(1, 1, instanceBuffer, instanceOffsets);

	uint32_t totalInstances = m_spiralScene.getInstanceCount();

	for (int lod = 0; lod < 4; lod++)
	{
		// get model for the LOD
		Model& lodModel = m_spiralScene.getModelLODSet(0).getLOD(lod);

		// bind vertex buffer for this LOD
		vk::Buffer vertexBuffers[] = { lodModel.getVertexBuffer() };
		vk::DeviceSize offsets[] = { 0 };
		cmdBuffer.bindVertexBuffers(0, 1, vertexBuffers, offsets);

		// bind index buffer
		cmdBuffer.bindIndexBuffer(lodModel.getIndexBuffer(), 0, vk::IndexType::eUint32);

		// push const - tell shader which LOD to render
		uint32_t currentLOD = lod;
		cmdBuffer.pushConstants(
			m_pipeline.getLayout(),
			vk::ShaderStageFlagBits::eVertex,
			0,
			sizeof(uint32_t),
			&currentLOD
		);

		// instanced draw
		// we draw all instances, but the shader will discard those not matching the LOD
		cmdBuffer.drawIndexed(
			lodModel.getIndexCount(),	// index number
			totalInstances,				// instance count
			0,							// first index	
			0,							// vertex offset
			0							// first instance
		);
	}

	ui.render(cmdBuffer);

	cmdBuffer.endRenderPass();
	cmdBuffer.end();

	// submit command buffer
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	vk::Semaphore waitSemaphores[] = { *imageAvailableSemaphores[currentFrame] };
	vk::Semaphore signalSemaphores[] = { *renderFinishedSemaphores[currentFrame] };

	vk::SubmitInfo submitInfo(
		1,
		waitSemaphores,
		waitStages,
		1,
		&cmdBuffer,
		1,
		signalSemaphores
	);

	m_device.getGraphicsQueue().submit(1, &submitInfo, *inFlightFence[currentFrame]);

	// present image
	vk::SwapchainKHR rawSwapchains[] = { m_swapchain.get().get() };

	vk::PresentInfoKHR presentInfo(
		1,						// waitSemaphoreCount
		signalSemaphores,       // pWaitSemaphores
		1,                      // swapchainCount
		rawSwapchains,          // pSwapchains
		&imgIdx,				// pImageIndices
		nullptr                 // pResults
	);

	m_device.getPresentQueue().presentKHR(presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void SpiralRenderer::recreateSwapchain()
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

void SpiralRenderer::recreateFramebuffers()
{
	m_framebuffer.cleanup();
	m_framebuffer.createFramebuffers(m_renderPass, m_swapchain);
}