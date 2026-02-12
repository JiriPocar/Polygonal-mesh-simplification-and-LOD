#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>

#include "RenderPass.hpp"
#include "FrameBuffer.hpp"
#include "CommandManager.hpp"
#include "UniformBuffer.hpp"
#include "Descriptors.hpp"
#include "../window.h"
#include "../core/Device.hpp"
#include "../core/Swapchain.hpp"
#include "../core/Pipeline.hpp"
#include "../resources/Model.hpp"
#include "../scene/Camera.hpp"
#include "../scene/Transform.hpp"
#include "../resources/DualModel.hpp"

class UserInterface;

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

	void drawFrame(const Camera& camera, const Transform& transform, UserInterface& ui);
	void drawSplitScreen(const Camera& camera, const Transform& transform, UserInterface& ui);
	
	void recreateSwapchain();

	void setModel(Model& newModel) { m_model = &newModel; }
	void setDualModel(DualModel& newDualModel) { m_dualModel = &newDualModel; }

	void setShowWireframe(bool show) { showWireframe = show; }
	void setWireframePipeline(Pipeline& pipeline) { m_wireframePipeline = &pipeline; }

	static const int MAX_FRAMES_IN_FLIGHT = 2;

private:
	void createSyncObjects();
	void cleanupSyncObjects();
	void recreateFramebuffers();

	void setupViewportScissor(vk::CommandBuffer cmd, vk::Extent2D extent, uint32_t width, int side);

	Device& m_device;
	Swapchain& m_swapchain;
	RenderPass& m_renderPass;
	Pipeline& m_pipeline;
	FrameBuffer& m_framebuffer;
	CommandManager& m_commandManager;
	Window& m_window;
	vk::SurfaceKHR m_surface;
	Model* m_model;
	UniformBuffer& m_uniformBuffer;
	Descriptor& m_descriptor;
	DualModel* m_dualModel;
	Pipeline* m_wireframePipeline = nullptr;

	vk::UniqueSemaphore imageAvailableSemaphores;
	vk::UniqueSemaphore renderFinishedSemaphores;
	vk::UniqueFence inFlightFence;

	bool framebufferResized = false;
	bool showWireframe = false;

	uint32_t currentFrame = 0;
};