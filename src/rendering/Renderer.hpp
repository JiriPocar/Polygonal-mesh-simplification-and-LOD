#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>

#include "../core/Device.hpp"
#include "../core/Swapchain.hpp"
#include "../core/Pipeline.hpp"
#include "RenderPass.hpp"
#include "FrameBuffer.hpp"
#include "CommandManager.hpp"

class Renderer {
public:
	Renderer(
		Device& device,
		Swapchain& swapchain,
		RenderPass& renderPass,
		Pipeline& pipeline,
		FrameBuffer& framebuffer,
		CommandManager& commandManager
	);

	~Renderer();

	void drawFrame();

private:
	void createSyncObjects();
	void cleanupSyncObjects();

	Device& m_device;
	Swapchain& m_swapchain;
	RenderPass& m_renderPass;
	Pipeline& m_pipeline;
	FrameBuffer& m_framebuffer;
	CommandManager& m_commandManager;

	vk::UniqueSemaphore imageAvailableSemaphores;
	vk::UniqueSemaphore renderFinishedSemaphores;
	vk::UniqueFence inFlightFence;
};