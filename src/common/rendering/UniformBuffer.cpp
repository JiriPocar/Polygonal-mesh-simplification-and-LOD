/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file UniformBuffer.cpp
 * @brief Uniform buffer management for Vulkan application.
 *
 * This file implements creating and managing Vulkan uniform buffers
 * 
 * Parts of the code may be inspired or adapted from:
 *		- Alexander Overvoorde's "Vulkan Tutorial"
 *			- @url https://vulkan-tutorial.com/Uniform_buffers/Descriptor_set_layout_and_buffer
 *			- @url https://github.com/Overv/VulkanTutorial
 */

#include "UniformBuffer.hpp"

UniformBuffer::UniformBuffer(Device& device, uint32_t maxFrames)
	: dev(device)
{
	buffers.resize(maxFrames);

	for (uint32_t i = 0; i < maxFrames; i++)
	{
		buffers[i] = std::make_unique<Buffer>(
			device,
			sizeof(UniformBufferObject),
			vk::BufferUsageFlagBits::eUniformBuffer,
			VMA_MEMORY_USAGE_CPU_TO_GPU,
			VMA_ALLOCATION_CREATE_MAPPED_BIT
		);
	}
}

void UniformBuffer::update(UniformBufferObject& ubo, uint32_t currentFrame)
{
	buffers[currentFrame]->copyData(&ubo, sizeof(ubo));
}

/* End of the UniformBuffer.cpp file */