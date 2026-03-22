#pragma once
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "../resources/Buffer.hpp"
#include "../core/Device.hpp"

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) glm::vec3 cameraPos;
	alignas(4)  uint32_t showLodColors;
};

class UniformBuffer {
public:
	UniformBuffer(Device& device, uint32_t maxFrames = 2);
	~UniformBuffer() = default;

	void update(UniformBufferObject& ubo, uint32_t currentFrame);

	vk::Buffer getBuffer(uint32_t currentFrame) const { return buffers[currentFrame]->getBuffer(); }

private:
	Device& dev;
	std::vector<std::unique_ptr<Buffer>> buffers;
};

