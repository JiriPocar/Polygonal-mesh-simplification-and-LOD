/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Renderer.cpp
 * @brief Source file for the base Renderer class.
 *
 * This file implements the Renderer class, which serves as a base for specific
 * renderers in the application. It manages common Vulkan rendering tasks such as
 * synchronization, command buffer management, and swapchain recreation on window resize.
 * 
 * Parts of the code may be inspired or adapted from:
 *		- Alexander Overvoorde's "Vulkan Tutorial"
 *			- @url https://vulkan-tutorial.com/
 *			- @url https://github.com/Overv/VulkanTutorial
 *		- Victor Blanco's "Vulkan Guide"
 *			- @url https://vkguide.dev/
 *			- @url https://github.com/vblanco20-1/vulkan-guide
 */

#include "Renderer.hpp"
#include <stdexcept>

Renderer::Renderer(Device& device, Swapchain& swapchain, RenderPass& renderPass,
    FrameBuffer& framebuffer, CommandManager& commandManager, Window& window, vk::SurfaceKHR surface)
    :   m_device(device),
        m_swapchain(swapchain),
        m_renderPass(renderPass),
        m_framebuffer(framebuffer),
        m_commandManager(commandManager),
        m_window(window),
        m_surface(surface)
{
    createSyncObjects();

    m_window.setResizeCallback([this](int width, int height) {
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

void Renderer::cleanupSyncObjects()
{
    m_device.operator*().waitIdle();
}

vk::CommandBuffer Renderer::beginFrame(uint32_t& outImgIdx)
{
    if (framebufferResized)
    {
        recreateSwapchain();
        return nullptr;
    }

    // wait for gpu to finish frame
    m_device.operator*().waitForFences(1, &*inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);

    try {
		// get next image from swapchain
        outImgIdx = m_swapchain.acquireNextImage(*imageAvailableSemaphores[currentFrame]);
    }
    catch (const vk::OutOfDateKHRError& e) {
        recreateSwapchain();
        return nullptr;
    }

	// if a previous frame is still using this image, wait for it to finish
    if (imagesInFlight[outImgIdx] != VK_NULL_HANDLE)
    {
        m_device.operator*().waitForFences(
            1,
            &imagesInFlight[outImgIdx],
            VK_TRUE,
            UINT64_MAX
        );
    }

	// mark image as now being in use by this frame
    imagesInFlight[outImgIdx] = *inFlightFence[currentFrame];

	// reset fence for current frame
    m_device.operator*().resetFences(1, &*inFlightFence[currentFrame]);

	// record command buffer
    vk::CommandBuffer cmdBuffer = m_commandManager.getCommandBuffer(outImgIdx);
    cmdBuffer.reset();

    vk::CommandBufferBeginInfo beginInfo;
    cmdBuffer.begin(beginInfo);

    return cmdBuffer;
}

void Renderer::endFrame(vk::CommandBuffer cmdBuffer, uint32_t imgIdx)
{
    cmdBuffer.end();

	// submit command buffer
    vk::PipelineStageFlags pipelineStageFlags[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    
    vk::Semaphore waitSemaphores[] = { *imageAvailableSemaphores[currentFrame] };
    vk::Semaphore signalSemaphores[] = { *renderFinishedSemaphores[currentFrame] };

    vk::SubmitInfo submitInfo(
        1,
        waitSemaphores,
        pipelineStageFlags,
        1,
        &cmdBuffer,
        1,
        signalSemaphores
    );

    m_device.getGraphicsQueue().submit(1, &submitInfo, *inFlightFence[currentFrame]);

    // present image
    vk::SwapchainKHR rawSwapchains[] = { m_swapchain.get().get() };
    vk::PresentInfoKHR presentInfo(
        1,
        signalSemaphores,
        1,
        rawSwapchains,
        &imgIdx,
        nullptr
    );

    try {
        m_device.getPresentQueue().presentKHR(presentInfo);
    }
    catch (const vk::OutOfDateKHRError& e) {
        framebufferResized = true;
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
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

    // prevent validation errors
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

void Renderer::recreateFramebuffers()
{
    m_framebuffer.cleanup();
    m_framebuffer.createFramebuffers(m_renderPass, m_swapchain);
}

/* End of the Renderer.cpp file */