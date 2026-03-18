#include "UniformBuffer.hpp"
#include <glm/gtc/matrix_transform.hpp>

UniformBuffer::UniformBuffer(Device& device, uint32_t maxFrames)
	: dev(device)
{
	buffers.resize(maxFrames);

	for (int i = 0; i < maxFrames; i++)
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
