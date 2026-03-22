#include "SpiralRenderer.hpp"
#include "../ui/ui.hpp"
#include "../common/core/SpiralComputePipeline.hpp"
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

	Texture* texture = m_spiralScene.getModelLODSet(0).getLOD(0).getTexture();
	if (texture)
	{
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_descriptor.updateTexture(i, *texture);
		}
	}
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
	m_spiralScene.updateLODs(camera.getPosition(), currentFrame, useGPULODCompute, useGPUSpiralCompute);

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

		ComputePushConstants pcs = {};
		pcs.totalInstances = m_spiralScene.config.instanceCount;
		pcs.lodDist0 = m_spiralScene.config.lodDist0 * m_spiralScene.config.lodDist0;
		pcs.lodDist1 = m_spiralScene.config.lodDist1 * m_spiralScene.config.lodDist1;
		pcs.lodDist2 = m_spiralScene.config.lodDist2 * m_spiralScene.config.lodDist2;
		pcs.lodDist3 = m_spiralScene.config.lodDist3 * m_spiralScene.config.lodDist3;
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

	vk::Buffer instanceBuffer[] = { useGPULODCompute ? m_spiralScene.getLODInstanceBuffer(currentFrame) : m_spiralScene.getInstanceBuffer(currentFrame) };
	vk::DeviceSize instanceOffsets[] = { 0 };
	cmdBuffer.bindVertexBuffers(1, 1, instanceBuffer, instanceOffsets);

	for (int lod = 0; lod < 4; lod++)
	{
		// get model for this LOD level
		Model& lodModel = m_spiralScene.getModelLODSet(0).getLOD(lod);

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

	// prevent validation errrors
	imageAvailableSemaphores.clear();
	renderFinishedSemaphores.clear();
	inFlightFence.clear();
	imagesInFlight.clear();

	m_swapchain.recreateOnResize(m_surface, width, height);
	recreateFramebuffers();

	// recreate sync objects, old were referencing old swapchain images
	createSyncObjects();
	framebufferResized = false;
}

void SpiralRenderer::recreateFramebuffers()
{
	m_framebuffer.cleanup();
	m_framebuffer.createFramebuffers(m_renderPass, m_swapchain);
}