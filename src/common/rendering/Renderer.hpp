/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Renderer.hpp
 * @brief Header file for the base Renderer class.
 *
 * This file implements the Renderer class, which serves as a base class for specific
 * renderers in the application. It manages common Vulkan rendering tasks such as
 * synchronization, command buffer management, and swapchain recreation on window resize.
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>

#include "RenderPass.hpp"
#include "FrameBuffer.hpp"
#include "CommandManager.hpp"
#include "../window.h"
#include "../core/Device.hpp"
#include "../core/Swapchain.hpp"

class Renderer {
public:
    Renderer(
        Device& device,
        Swapchain& swapchain,
        RenderPass& renderPass,
        FrameBuffer& framebuffer,
        CommandManager& commandManager,
        Window& window,
        vk::SurfaceKHR surface
    );
    virtual ~Renderer();

	// recreates the swapchain and all dependent resources
    void recreateSwapchain();

    // getters
	uint32_t getCurrentFrame() const { return currentFrame; }

    static const int MAX_FRAMES_IN_FLIGHT = 2;

protected:
    /**
	* @brief Begins recording a command buffer for the current frame
    *        and acquires the next image from the swapchain.
    * 
	* @param outImgIdx Output parameter to receive the index of the acquired swapchain image
    * 
	* @return vk::CommandBuffer ready command buffer
    */
    vk::CommandBuffer beginFrame(uint32_t& outImgIdx);

    /**
	* @brief Ends recording of the command buffer for the current frame,
    *        submits it to the graphics queue, and presents the swapchain image.
    * 
	* @param cmd The command buffer that was recorded for the current frame
	* @param imgIdx Index of the swapchain image that was acquired for the current frame
    */
    void endFrame(vk::CommandBuffer cmd, uint32_t imgIdx);

    Device& m_device;
    Swapchain& m_swapchain;
    RenderPass& m_renderPass;
    FrameBuffer& m_framebuffer;
    CommandManager& m_commandManager;
    Window& m_window;
    vk::SurfaceKHR m_surface;

    // swaps between 0/1 to track current frame in flight
    uint32_t currentFrame = 0;
    bool framebufferResized = false;

private:
    void createSyncObjects();
    void cleanupSyncObjects();
    void recreateFramebuffers();

    std::vector<vk::UniqueSemaphore> imageAvailableSemaphores;
    std::vector<vk::UniqueSemaphore> renderFinishedSemaphores;
    std::vector<vk::UniqueFence> inFlightFence;
    std::vector<vk::Fence> imagesInFlight;
};

/* End of the Renderer.hpp file */