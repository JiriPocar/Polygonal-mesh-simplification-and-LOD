#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>

#include "../core/Device.hpp"
#include "../core/Swapchain.hpp"
#include "../core/Pipeline.hpp"
#include "../window.h"
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
		CommandManager& commandManager,
		Window& window,
		vk::SurfaceKHR surface
	);

	~Renderer();

	void drawFrame();
	void recreateSwapchain();

private:
	void createSyncObjects();
	void cleanupSyncObjects();
	void recreateFramebuffers();

	Device& m_device;
	Swapchain& m_swapchain;
	RenderPass& m_renderPass;
	Pipeline& m_pipeline;
	FrameBuffer& m_framebuffer;
	CommandManager& m_commandManager;
	Window& m_window;
	vk::SurfaceKHR m_surface;

	vk::UniqueSemaphore imageAvailableSemaphores;
	vk::UniqueSemaphore renderFinishedSemaphores;
	vk::UniqueFence inFlightFence;

	bool framebufferResized = false;
};