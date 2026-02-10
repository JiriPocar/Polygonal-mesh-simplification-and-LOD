#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>
#include <array>

#include "RenderPass.hpp"
#include "FrameBuffer.hpp"
#include "CommandManager.hpp"
#include "UniformBuffer.hpp"
#include "Descriptors.hpp"
#include "../window.h"
#include "../core/Device.hpp"
#include "../core/Swapchain.hpp"
#include "../core/SpiralPipeline.hpp"
#include "../scene/Camera.hpp"
#include "../scene/SpiralScene.hpp"

class UserInterface;

class SpiralRenderer {
public:
    SpiralRenderer(
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
    );

    ~SpiralRenderer();

    void drawFrame(const Camera& camera, UserInterface& ui);
    void recreateSwapchain();
	void setWireframePipeline(SpiralPipeline& pipeline) { m_wireframePipeline = &pipeline; }
	void setShowWireframe(bool show) { showWireframe = show; }

    static const int MAX_FRAMES_IN_FLIGHT = 2;

private:
    void createSyncObjects();
    void cleanupSyncObjects();
    void recreateFramebuffers();

    Device& m_device;
    Swapchain& m_swapchain;
    RenderPass& m_renderPass;
    SpiralPipeline& m_pipeline;
    SpiralPipeline* m_wireframePipeline = nullptr;
    FrameBuffer& m_framebuffer;
    CommandManager& m_commandManager;
    Window& m_window;
    vk::SurfaceKHR m_surface;
    SpiralScene& m_spiralScene;
    UniformBuffer& m_uniformBuffer;
    Descriptor& m_descriptor;

    std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
    std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
    std::vector<vk::UniqueFence> inFlightFence;
    std::vector<vk::Fence> imagesInFlight;

    bool showWireframe = false;

	// swaps between 0/1 to track current frame in flight
    uint32_t currentFrame = 0;

    bool framebufferResized = false;
};