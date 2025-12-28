#include "RenderPass.hpp"
#include "../core/Device.hpp"
#include <stdexcept>
#include <iostream>

RenderPass::RenderPass(const Device& device, vk::Format swapchainImageFormat)
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

	// subpass with the color attachment
	vk::SubpassDescription subpass(
		{},									// flags
		vk::PipelineBindPoint::eGraphics,	// graphics pipeline
		0, nullptr,							// input attachments
		1, &colorAttachmentRef,			    // color attachments
		nullptr,							// no resolve attachments
		nullptr,							// no depth/stencil attachment
		0, nullptr							// no preserve attachments
	);

	vk::SubpassDependency dependency(
		VK_SUBPASS_EXTERNAL,								// source subpass
		0,													// dest subpass
		vk::PipelineStageFlagBits::eColorAttachmentOutput,	// source stage
		vk::PipelineStageFlagBits::eColorAttachmentOutput,	// dest stage
		{},													// no source access mask
		vk::AccessFlagBits::eColorAttachmentWrite,          // dest access mask
		{}													// no dependency flags
	);

	vk::RenderPassCreateInfo renderPassInfo(
		{},							// flags
		1, &colorAttachment,		// attachments
		1, &subpass,				// subpasses
		1, &dependency				// dependencies
	);

	renderPass = renderPassDevice.operator*().createRenderPassUnique(renderPassInfo);
}