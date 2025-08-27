#include "FrameBuffer.hpp"
#include "../core/Device.hpp"
#include "../core/Swapchain.hpp"
#include "RenderPass.hpp"
#include <stdexcept>
#include <iostream>

FrameBuffer::FrameBuffer(const Device& device, const RenderPass& renderPass, const Swapchain& swapchain)
	: dev(device)
{
	createFramebuffers(renderPass, swapchain);
}

void FrameBuffer::createFramebuffers(const RenderPass& renderPass, const Swapchain& swapchain)
{
	auto const& imageViews = swapchain.getImageViews();
	framebuffers.reserve(imageViews.size());

	vk::Extent2D extent = swapchain.getExtent();

	for (size_t i = 0; i < imageViews.size(); i++)
	{
		// for each image view create a framebuffer
		vk::ImageView attachments[] = {
			*imageViews[i]
		};

		vk::FramebufferCreateInfo fbInfo(
			{},					// flags
			renderPass.get(),	// render pass
			1,					// attachment count
			attachments,		// attachment array
			extent.width,		// width
			extent.height,	    // height
			1					// layers	
		);

		try {
			framebuffers.push_back(dev.operator*().createFramebufferUnique(fbInfo));
		}
		catch (const std::exception& e) {
			throw std::runtime_error(std::string("Failed to create framebuffer: ") + e.what());
		}
	}
}