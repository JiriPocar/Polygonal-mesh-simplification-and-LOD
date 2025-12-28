#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>

class Device;

class CommandManager {
public:
	CommandManager(const Device& device);
	~CommandManager() = default;

	vk::CommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(vk::CommandBuffer commandBuffer);

	void createCommandBuffers(uint32_t count);
	vk::CommandBuffer getCommandBuffer(uint32_t index) const;

private:
	const Device& dev;
	vk::UniqueCommandPool commandPool;
	std::vector<vk::UniqueCommandBuffer> commandBuffers;
};