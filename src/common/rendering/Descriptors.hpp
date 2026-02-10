#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include "UniformBuffer.hpp"
#include "../core/Device.hpp"

class Texture;

class Descriptor {
public:
	Descriptor(Device& device, UniformBuffer& uniformBuffer, uint32_t maxFrames = 2);
	~Descriptor() = default;

	vk::DescriptorSet get(uint32_t frame) const { return *descriptorSets[frame]; }
	vk::DescriptorSetLayout getLayout() const { return *descriptorSetLayout; }

	void updateTexture(uint32_t frame, Texture& texture);

private:
	void createDescriptorSetLayout();
	void createDescriptorPool(uint32_t maxFrames);
	void createDescriptorSets(UniformBuffer& uniformBuffer, uint32_t maxFrames);

	Device& dev;
	vk::UniqueDescriptorSetLayout descriptorSetLayout;
	vk::UniqueDescriptorPool descriptorPool;
	std::vector<vk::UniqueDescriptorSet> descriptorSets; // per frame
};
