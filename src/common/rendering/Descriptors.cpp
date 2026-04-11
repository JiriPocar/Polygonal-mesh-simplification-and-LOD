/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Descriptors.cpp
 * @brief Descriptor set management for Vulkan application.
 *
 * This file contains the implementation of the Descriptor class,
 * which is responsible for creating and managing Vulkan descriptor
 * sets and descriptor set layouts for both graphics and compute pipelines.
 * 
 * Parts of the code may be inspired or adapted from:
 *		- Alexander Overvoorde's "Vulkan Tutorial"
 *			- @url https://vulkan-tutorial.com/
 *			- @url https://github.com/Overv/VulkanTutorial
 *		- Victor Blanco's "Vulkan Guide"
 *			- @url https://vkguide.dev/
 *			- @url https://github.com/vblanco20-1/vulkan-guide
 */

#include "Descriptors.hpp"
#include "../common/resources/Textures.hpp"
#include "../apps/demo/SpiralScene.hpp"
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
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
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
	// need maxFrames descriptor sets, each with 1 ubo and 1 sampler for graphics, and 1 ubo + 3 storage buffers for compute
	std::array<vk::DescriptorPoolSize, 3> poolSizes = {
		vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, maxFrames * 2), // ubo for graphics and compute
		vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, maxFrames), // sampler for graphics
		vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, maxFrames * 3) // storage buffers for compute shader (instance data, culled instance data, indirect commands)
	};

	vk::DescriptorPoolCreateInfo poolInfo(
		vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		maxFrames * 2, // maxFrames for graphics + maxFrames for compute
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

void Descriptor::createComputeDescriptors(SpiralScene& scene, uint32_t maxFrames)
{
	// binding 0 - UBO
	vk::DescriptorSetLayoutBinding uboBinding(
		0,
		vk::DescriptorType::eUniformBuffer,
		1,
		vk::ShaderStageFlagBits::eCompute
	);

	// binding 1 - input raw instance data (Storage Buffer)
	vk::DescriptorSetLayoutBinding inBinding(
		1,
		vk::DescriptorType::eStorageBuffer,
		1,
		vk::ShaderStageFlagBits::eCompute
	);

	// binding 2 - output sorted instance data (Storage Buffer)
	vk::DescriptorSetLayoutBinding outBinding(
		2,
		vk::DescriptorType::eStorageBuffer,
		1,
		vk::ShaderStageFlagBits::eCompute
	);

	// binding 3 - indirect commands (Storage Buffer)
	vk::DescriptorSetLayoutBinding indBinding(
		3,
		vk::DescriptorType::eStorageBuffer,
		1,
		vk::ShaderStageFlagBits::eCompute
	);

	std::array bindings = { uboBinding, inBinding, outBinding, indBinding };
	vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings.size(), bindings.data());
	computeDescriptorSetLayout = dev.operator*().createDescriptorSetLayoutUnique(layoutInfo);

	std::vector<vk::DescriptorSetLayout> layouts(maxFrames, *computeDescriptorSetLayout);
	vk::DescriptorSetAllocateInfo allocInfo(*descriptorPool, maxFrames, layouts.data());
	computeDescriptorSets = dev.operator*().allocateDescriptorSetsUnique(allocInfo);

	// write descriptors for each frame
	for (uint32_t i = 0; i < maxFrames; i++) {
		vk::DescriptorBufferInfo uniformBuffer(scene.getUniformBuffer().getBuffer(i), 0, sizeof(UniformBufferObject));
		vk::DescriptorBufferInfo inputInstanceBuffer(scene.getInstanceBuffer(i), 0, VK_WHOLE_SIZE);
		vk::DescriptorBufferInfo outputInstanceBuffer(scene.getLODInstanceBuffer(i), 0, VK_WHOLE_SIZE);
		vk::DescriptorBufferInfo indirectBuffer(scene.getIndirectBuffer(i), 0, VK_WHOLE_SIZE);

		std::array<vk::WriteDescriptorSet, 4> writes = {
			vk::WriteDescriptorSet(
				*computeDescriptorSets[i],
				0,
				0,
				1,
				vk::DescriptorType::eUniformBuffer,
				nullptr,
				&uniformBuffer,
				nullptr
			),
			vk::WriteDescriptorSet(
				*computeDescriptorSets[i],
				1,
				0,
				1,
				vk::DescriptorType::eStorageBuffer,
				nullptr,
				&inputInstanceBuffer,
				nullptr
			),
			vk::WriteDescriptorSet(
				*computeDescriptorSets[i],
				2,
				0,
				1,
				vk::DescriptorType::eStorageBuffer,
				nullptr,
				&outputInstanceBuffer,
				nullptr
			),
			vk::WriteDescriptorSet(
				*computeDescriptorSets[i],
				3,
				0,
				1,
				vk::DescriptorType::eStorageBuffer,
				nullptr,
				&indirectBuffer,
				nullptr
			)
		};
		dev.operator*().updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);
	}
}

/* End of the Descriptors.cpp file */