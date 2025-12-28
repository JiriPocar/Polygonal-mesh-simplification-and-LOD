#include "Descriptors.hpp"

Descriptor::Descriptor(const Device& device, const UniformBuffer& uniformBuffer)
	: dev(device)
{
	createDescriptorSetLayout();
	createDescriptorPool();
	createDescriptorSet(uniformBuffer);
}

void Descriptor::createDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding uboLayoutBinding(
		0,
		vk::DescriptorType::eUniformBuffer,
		1,
		vk::ShaderStageFlagBits::eVertex,
		nullptr
	);

	vk::DescriptorSetLayoutCreateInfo layoutInfo(
		{},
		1,
		&uboLayoutBinding
	);

	descriptorSetLayout = dev.operator*().createDescriptorSetLayoutUnique(layoutInfo);
}

void Descriptor::createDescriptorPool()
{
	vk::DescriptorPoolSize poolSize(
		vk::DescriptorType::eUniformBuffer,
		1
	);

	vk::DescriptorPoolCreateInfo poolInfo(
		{},
		1,
		1,
		&poolSize
	);

	descriptorPool = dev.operator*().createDescriptorPoolUnique(poolInfo);
}

void Descriptor::createDescriptorSet(const UniformBuffer& uniformBuffer)
{
	vk::DescriptorSetAllocateInfo allocInfo(
		*descriptorPool,
		1,
		&*descriptorSetLayout
	);

	descriptorSet = std::move(dev.operator*().allocateDescriptorSetsUnique(allocInfo).front());

	vk::DescriptorBufferInfo bufferInfo(
		uniformBuffer.getBuffer(),
		0,
		sizeof(UniformBufferObject)
	);

	vk::WriteDescriptorSet descriptorWrite(
		*descriptorSet,
		0,
		0,
		1,
		vk::DescriptorType::eUniformBuffer,
		nullptr,
		&bufferInfo,
		nullptr
	);

	dev.operator*().updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
}
