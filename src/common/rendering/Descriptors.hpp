#pragma once

#include <vulkan/vulkan.hpp>
#include <memory>
#include "UniformBuffer.hpp"
#include "../core/Device.hpp"

class Descriptor {
public:
	Descriptor(const Device& device, const UniformBuffer& uniformBuffer);
	~Descriptor() = default;

	vk::DescriptorSet get() const { return *descriptorSet; }
	vk::DescriptorSetLayout getLayout() const { return *descriptorSetLayout; }

private:
	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSet(const UniformBuffer& uniformBuffer);

	const Device& dev;
	vk::UniqueDescriptorSetLayout descriptorSetLayout;
	vk::UniqueDescriptorPool descriptorPool;
	vk::UniqueDescriptorSet descriptorSet;
};
