#include "UniformBuffer.hpp"
#include <glm/gtc/matrix_transform.hpp>

UniformBuffer::UniformBuffer(const Device& device)
	: dev(device)
{
	buffer = std::make_unique<Buffer>(
		device,
		sizeof(UniformBufferObject),
		vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	);
}

void UniformBuffer::update(const UniformBufferObject& ubo)
{
	buffer->copyData(&ubo, sizeof(ubo));
}
