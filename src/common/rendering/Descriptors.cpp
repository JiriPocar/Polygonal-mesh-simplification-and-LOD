#include "Descriptors.hpp"
#include "../common/resources/Textures.hpp"
#include <array>

Descriptor::Descriptor(Device& device, UniformBuffer& uniformBuffer, uint32_t maxFrames)
	: dev(device)
{
	createDescriptorSetLayout();
	createDescriptorPool(maxFrames);
	createDescriptorSets(uniformBuffer, maxFrames);
}

void Descriptor::createDescriptorSetLayout()
{
	// binding 0, ubo vertex shader
	vk::DescriptorSetLayoutBinding uboLayoutBinding(
		0,
		vk::DescriptorType::eUniformBuffer,
		1,
		vk::ShaderStageFlagBits::eVertex,
		nullptr
	);

	// binding 1, sampler fragment shader
	vk::DescriptorSetLayoutBinding samplerLayoutBinding(
		1,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment,
		nullptr
	);

	std::array<vk::DescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

	vk::DescriptorSetLayoutCreateInfo layoutInfo(
		{},
		bindings.size(),
		bindings.data()
	);

	descriptorSetLayout = dev.operator*().createDescriptorSetLayoutUnique(layoutInfo);
}

void Descriptor::createDescriptorPool(uint32_t maxFrames)
{
	// we need maxFrames descriptor sets, each with 1 ubo and 1 sampler
	std::array<vk::DescriptorPoolSize, 2> poolSizes = {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, maxFrames),
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, maxFrames)
	};

	vk::DescriptorPoolCreateInfo poolInfo(
		vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		maxFrames,
		poolSizes.size(),
		poolSizes.data()
	);

	descriptorPool = dev.operator*().createDescriptorPoolUnique(poolInfo);
}

void Descriptor::createDescriptorSets(UniformBuffer& uniformBuffer, uint32_t maxFrames)
{
	std::vector<vk::DescriptorSetLayout> layouts(maxFrames, *descriptorSetLayout);

	vk::DescriptorSetAllocateInfo allocInfo(
		*descriptorPool,
		maxFrames,
		layouts.data()
	);

	descriptorSets = dev.operator*().allocateDescriptorSetsUnique(allocInfo);

	for (int i = 0; i < maxFrames; i++)
	{
		vk::DescriptorBufferInfo bufferInfo(
			uniformBuffer.getBuffer(i),
			0,
			sizeof(UniformBufferObject)
		);

		vk::WriteDescriptorSet descriptorSetWrite(
			*descriptorSets[i],
			0,
			0,
			1,
			vk::DescriptorType::eUniformBuffer,
			nullptr,
			&bufferInfo,
			nullptr
		);

		dev.operator*().updateDescriptorSets(1, &descriptorSetWrite, 0, nullptr);
	}
}

void Descriptor::updateTexture(uint32_t frame, Texture& texture)
{
	vk::DescriptorImageInfo imageInfo(
		texture.getSampler(),
		texture.getImageView(),
		vk::ImageLayout::eShaderReadOnlyOptimal
	);

	vk::WriteDescriptorSet descriptorSetWrite(
		*descriptorSets[frame],
		1,
		0,
		1,
		vk::DescriptorType::eCombinedImageSampler,
		&imageInfo,
		nullptr,
		nullptr
	);

	dev.operator*().updateDescriptorSets(1, &descriptorSetWrite, 0, nullptr);
}
