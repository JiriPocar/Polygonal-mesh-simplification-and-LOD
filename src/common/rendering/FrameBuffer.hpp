/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Framebuffer.hpp
 * @brief Framebuffer management for Vulkan application.
 *
 * This file contains the implementation of the FrameBuffer class,
 * which is responsible for creating and managing Vulkan framebuffers
 * for each swapchain image view and the depth image view.
 */

#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>

class Device;
class Swapchain;
class RenderPass;

class FrameBuffer {
public:
	FrameBuffer(Device& device, RenderPass& renderPass, Swapchain& swapchain);
	~FrameBuffer() = default;

	/**
	* @brief Creates framebuffers for each swapchain image view and the depth image view.
	* 
	* @param renderPass The render pass that the framebuffers will be compatible with
	* @param swapchain The swapchain containing the image views and depth image view to use as attachments
	*/
	void createFramebuffers(RenderPass& renderPass, Swapchain& swapchain);

	// cleanup framebuffers before recreating them
	void cleanup() { framebuffers.clear(); }

	// getters
	vk::Framebuffer getFrameBufferAt(size_t index) const { return framebuffers.at(index).get(); }
	 
private:
	const Device& dev;
	std::vector<vk::UniqueFramebuffer> framebuffers;
};

/* End of the FrameBuffer.hpp file */