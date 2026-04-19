/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file CommandManager.cpp
 * @brief Command buffer management for Vulkan application.
 *
 * This file contains the implementation of the CommandManager class,
 * which is responsible for creating and managing Vulkan command buffers.
 * Provides utility functions for common command buffer operations.
 * 
 * Parts of the code may be inspired or adapted from:
 *		- Alexander Overvoorde's "Vulkan Tutorial"
 *			- @url https://vulkan-tutorial.com/
 *			- @url https://github.com/Overv/VulkanTutorial
 *		- Victor Blanco's "Vulkan Guide"
 *			- @url https://vkguide.dev/
 *			- @url https://github.com/vblanco20-1/vulkan-guide
 */

#include "CommandManager.hpp"
#include "../core/Device.hpp"
#include <stdexcept>
#include <iostream>

CommandManager::CommandManager(Device& device) : dev(device)
{
	vk::CommandPoolCreateInfo info(
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		device.getGraphicsQueueFamily()
	);

	commandPool = device.operator*().createCommandPoolUnique(info);
}

void CommandManager::createCommandBuffers(uint32_t count)
{
	vk::CommandBufferAllocateInfo info(
		*commandPool,
		vk::CommandBufferLevel::ePrimary,
		count
	);

	commandBuffers = dev.operator*().allocateCommandBuffersUnique(info);
	if (commandBuffers.size() != count) {
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}

vk::CommandBuffer CommandManager::getCommandBuffer(uint32_t index) const
{
	if (index >= commandBuffers.size()) {
		throw std::runtime_error("Command buffer index out of range!");
	}
	return *commandBuffers[index];
}

vk::CommandBuffer CommandManager::beginSingleTimeCommands()
{
	vk::CommandBufferAllocateInfo info(
		*commandPool,						// command pool to allocate from
		vk::CommandBufferLevel::ePrimary,	// primary command buffer
		1									// allocate one command buffer
	);

	auto commandBuffer = std::move(dev.operator*().allocateCommandBuffers(info)[0]);

	// one-time submit flag
	vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	commandBuffer.begin(beginInfo);
	return commandBuffer;
}

void CommandManager::endSingleTimeCommands(vk::CommandBuffer commandBuffer)
{
	commandBuffer.end();
	vk::SubmitInfo submitInfo(
		0, nullptr, nullptr,	// no semaphore waits
		1, &commandBuffer,		// submit the command buffer
		0, nullptr				// no semaphore signals
	);

	// send to GPU
	dev.getGraphicsQueue().submit(1, &submitInfo, nullptr);
	// wait for the GPU to finish executing the command buffer
	dev.getGraphicsQueue().waitIdle();
	// free the command buffer back to the pool
	dev.operator*().freeCommandBuffers(*commandPool, 1, &commandBuffer);
}

void CommandManager::transitionImageLayout(vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	vk::CommandBuffer cmdBuffer = beginSingleTimeCommands();

	vk::ImageMemoryBarrier barrier(
		{},							// no access masks
		{},
		oldLayout,					// which layout to which
		newLayout,
		VK_QUEUE_FAMILY_IGNORED,	// ignore queue family ownership transfer
		VK_QUEUE_FAMILY_IGNORED,
		image,						// the image to transition
		vk::ImageSubresourceRange(	// the subresource range to transition
			vk::ImageAspectFlagBits::eColor,	// texture with color
			0, 1, 0, 1							// mip levels, array layers
		)
	);

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	// data transfer
	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	// shader read
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else
	{
		throw std::invalid_argument("Unsupported layout transition!");
	}

	cmdBuffer.pipelineBarrier(
		sourceStage,		// wait for the previous stage to finish
		destinationStage,	// the stage that will wait for the barrier
		{},					// no dependency flags
		0, nullptr,			// no memory barriers
		0, nullptr,			// no buffer memory barriers
		1, &barrier			// the image memory barrier to execute	
	);

	endSingleTimeCommands(cmdBuffer);
}

void CommandManager::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
{
	vk::CommandBuffer cmdBuffer = beginSingleTimeCommands();

	vk::BufferImageCopy region(
		0, 0, 0,								// buffer offset, row length, image height
		vk::ImageSubresourceLayers(				// the image subresource to copy to
			vk::ImageAspectFlagBits::eColor,
			0, 0, 1		// mip level, base array layer, layer count
		),
		vk::Offset3D(0, 0, 0),
		vk::Extent3D(width, height, 1)
	);

	cmdBuffer.copyBufferToImage(
		buffer,									// the buffer to copy from
		image,									// the image to copy to	
		vk::ImageLayout::eTransferDstOptimal,	// expect image to be in transfer destination layout
		1,										// region count
		&region									// the region to copy	
	);

	endSingleTimeCommands(cmdBuffer);
}

/* End of the CommandManager.cpp file */
