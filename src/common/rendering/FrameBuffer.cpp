/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Framebuffer.hpp
 * @brief Framebuffer management for Vulkan application.
 *
 * This file contains the implementation of the FrameBuffer class,
 * which is responsible for creating and managing Vulkan framebuffers
 * for each swapchain image view and the depth image view.
 * 
 * Parts of the code may be inspired or adapted from:
 *		- Alexander Overvoorde's "Vulkan Tutorial"
 *			- @url https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Framebuffers
 *			- @url https://github.com/Overv/VulkanTutorial
 */

#include "FrameBuffer.hpp"
#include "common/core/Device.hpp"
#include "common/core/Swapchain.hpp"
#include "RenderPass.hpp"
#include <stdexcept>
#include <iostream>

FrameBuffer::FrameBuffer(Device& device, RenderPass& renderPass, Swapchain& swapchain)
	: dev(device)
{
	createFramebuffers(renderPass, swapchain);
}

void FrameBuffer::createFramebuffers(RenderPass& renderPass, Swapchain& swapchain)
{
	cleanup();

	auto const& imageViews = swapchain.getImageViews();
	framebuffers.reserve(imageViews.size());

	vk::Extent2D extent = swapchain.getExtent();

	for (size_t i = 0; i < imageViews.size(); i++)
	{
		// for each image view create a framebuffer
		std::array<vk::ImageView, 2> attachments = {
			*imageViews[i],
			swapchain.getDepthImageView()
		};

		vk::FramebufferCreateInfo fbInfo(
			{},					// flags
			renderPass.get(),	// render pass
			static_cast<uint32_t>(attachments.size()),	// attachment count
			attachments.data(),	// attachment array
			extent.width,		// width
			extent.height,	    // height
			1					// layers	
		);

		framebuffers.push_back(dev.operator*().createFramebufferUnique(fbInfo));
	}
}

/* End of the FrameBuffer.cpp file */