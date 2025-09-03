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
#include "../resources/Model.hpp"
#include "UniformBuffer.hpp"
#include "Descriptors.hpp"
#include "../scene/Camera.hpp"
#include "../scene/Transform.hpp"

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
		vk::SurfaceKHR surface,
		Model& model,
		UniformBuffer& uniformBuffer,
		Descriptor& descriptor
	);

	~Renderer();

	void drawFrame(const Camera& camera, const Transform& transform);
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
	Model& m_model;
	UniformBuffer& m_uniformBuffer;
	Descriptor& m_descriptor;

	vk::UniqueSemaphore imageAvailableSemaphores;
	vk::UniqueSemaphore renderFinishedSemaphores;
	vk::UniqueFence inFlightFence;

	bool framebufferResized = false;
};