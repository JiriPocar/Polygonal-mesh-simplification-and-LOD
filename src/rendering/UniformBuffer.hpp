#pragma once
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <memory>
#include "../resources/Buffer.hpp"
#include "../core/Device.hpp"

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class UniformBuffer {
public:
	UniformBuffer(const Device& device);
	~UniformBuffer() = default;

	void update(const UniformBufferObject& ubo);
	vk::Buffer getBuffer() const { return buffer->getBuffer(); }

private:
	const Device& dev;
	std::unique_ptr<Buffer> buffer;
};

