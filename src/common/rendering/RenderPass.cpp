/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file RenderPass.cpp
 * @brief Render pass management for Vulkan application.
 *
 * This file creates render pass on construction.
 * 
 * Parts of the code may be inspired or adapted from:
 *		- Alexander Overvoorde's "Vulkan Tutorial"
 *			- @url https://vulkan-tutorial.com/Drawing_a_triangle/Graphics_pipeline_basics/Render_passes
 *			- @url https://github.com/Overv/VulkanTutorial
 */

#include "RenderPass.hpp"
#include "common/core/Device.hpp"
#include <stdexcept>
#include <iostream>

RenderPass::RenderPass(Device& device, vk::Format swapchainImageFormat)
	: renderPassDevice(device), format(swapchainImageFormat)
{
	createRenderpass(swapchainImageFormat);
}

void RenderPass::createRenderpass(vk::Format swapchainImageFormat)
{
	vk::AttachmentDescription colorAttachment(
		{},									// flags
		swapchainImageFormat,
		vk::SampleCountFlagBits::e1,		// no multisampling
		vk::AttachmentLoadOp::eClear,		// clear before render
		vk::AttachmentStoreOp::eStore,		// store for presentation
		vk::AttachmentLoadOp::eDontCare,	// no stencil
		vk::AttachmentStoreOp::eDontCare,	
		vk::ImageLayout::eUndefined,		// before render
		vk::ImageLayout::ePresentSrcKHR		// after render
	);

	vk::AttachmentReference colorAttachmentRef(
		0,
		vk::ImageLayout::eColorAttachmentOptimal
	);

	vk::Format depthFormat = renderPassDevice.findDepthFormat();

	vk::AttachmentDescription depthAttachment(
		{},									// flags
		depthFormat,						// format
		vk::SampleCountFlagBits::e1,		// no multisampling
		vk::AttachmentLoadOp::eClear,		// clear before render
		vk::AttachmentStoreOp::eDontCare,	// no need to store depth data
		vk::AttachmentLoadOp::eDontCare,	// no stencil
		vk::AttachmentStoreOp::eDontCare,	// no stencil
		vk::ImageLayout::eUndefined,		// before render
		vk::ImageLayout::eDepthStencilAttachmentOptimal	// after render
	);

	vk::AttachmentReference depthAttachmentRef(
		1,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	);

	// subpass with the color attachment
	vk::SubpassDescription subpass(
		{},									// flags
		vk::PipelineBindPoint::eGraphics,	// graphics pipeline
		0, nullptr,							// input attachments
		1, &colorAttachmentRef,			    // color attachments
		nullptr,							// no resolve attachments
		&depthAttachmentRef,				// no depth/stencil attachment
		0, nullptr							// no preserve attachments
	);

	vk::SubpassDependency dependency(
		VK_SUBPASS_EXTERNAL,	// source subpass
		0,						// dest subpass
		vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,	// source stage
		vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests,	// dest stage
		vk::AccessFlagBits::eDepthStencilAttachmentWrite,	// source access mask
		vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,		// dest access mask
		{}	// no dependency flags
	);

	std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

	vk::RenderPassCreateInfo renderPassInfo(
		{},											// flags
		static_cast<uint32_t>(attachments.size()),	// attachments
		attachments.data(),							// attachments
		1, &subpass,								// subpasses
		1, &dependency								// dependencies
	);

	renderPass = renderPassDevice.operator*().createRenderPassUnique(renderPassInfo);
}

/* End of the Renderer.cpp file */