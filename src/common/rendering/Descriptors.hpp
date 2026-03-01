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
	vk::DescriptorSet getCompute(uint32_t frame) const { return *computeDescriptorSets[frame]; };

	vk::DescriptorSetLayout getLayout() const { return *descriptorSetLayout; }
	vk::DescriptorSetLayout getComputeLayout() const { return *computeDescriptorSetLayout; }

	void updateTexture(uint32_t frame, Texture& texture);
	void createComputeDescriptors(class SpiralScene& scene, uint32_t maxFrames = 2);

private:
	void createDescriptorSetLayout();
	void createDescriptorPool(uint32_t maxFrames);
	void createDescriptorSets(UniformBuffer& uniformBuffer, uint32_t maxFrames);

	Device& dev;
	vk::UniqueDescriptorSetLayout descriptorSetLayout;
	vk::UniqueDescriptorSetLayout computeDescriptorSetLayout;
	vk::UniqueDescriptorPool descriptorPool;
	std::vector<vk::UniqueDescriptorSet> computeDescriptorSets; // per frame
	std::vector<vk::UniqueDescriptorSet> descriptorSets; // per frame
};
