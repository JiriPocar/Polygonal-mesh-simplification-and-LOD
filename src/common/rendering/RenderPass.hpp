/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file RenderPass.hpp
 * @brief Render pass management for Vulkan application.
 *
 * This file implements the RenderPass class, which is responsible for creating a Vulkan render pass
 */

#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>

class Device;

class RenderPass {
public:
	RenderPass(Device& device, vk::Format swapchainImageFormat);
	~RenderPass() = default;

	// getters
	vk::RenderPass get() const { return renderPass.operator*(); }
	vk::Format getFormat() const { return format; }

private:
	/**
	* @brief Creates a Vulkan render pass.
	* 
	* @param swapchainImageFormat The format of the swapchain images to use for the color attachment
	*/
	void createRenderpass(vk::Format swapchainImageFormat);

	vk::Format format;
	vk::UniqueRenderPass renderPass;
	Device& renderPassDevice;
};

/* End of the Renderer.hpp file */