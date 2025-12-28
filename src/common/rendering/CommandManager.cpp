#include "CommandManager.hpp"
#include "../core/Device.hpp"
#include <stdexcept>
#include <iostream>

CommandManager::CommandManager(const Device& device) : dev(device)
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
		*commandPool,
		vk::CommandBufferLevel::ePrimary,
		1
	);

	auto commandBuffer = std::move(dev.operator*().allocateCommandBuffersUnique(info)[0]);

	vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	commandBuffer->begin(beginInfo);
	return *commandBuffer;
}

void CommandManager::endSingleTimeCommands(vk::CommandBuffer commandBuffer)
{
	commandBuffer.end();
	vk::SubmitInfo submitInfo(
		0, nullptr, nullptr,
		1, &commandBuffer,
		0, nullptr
	);
	dev.getGraphicsQueue().submit(1, &submitInfo, nullptr);
	dev.getGraphicsQueue().waitIdle();
}
