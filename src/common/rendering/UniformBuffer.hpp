/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file UniformBuffer.hpp
 * @brief Uniform buffer management for Vulkan application.
 *
 * This file implements the UniformBuffer class, creating and managing Vulkan uniform buffers
 */

#pragma once
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "common/resources/Buffer.hpp"
#include "common/core/Device.hpp"

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
	alignas(16) glm::vec3 cameraPos;
	alignas(4)  uint32_t showLodColors; // TODO move this to push constants
};

class UniformBuffer {
public:
	UniformBuffer(Device& device, uint32_t maxFrames = 2);
	~UniformBuffer() = default;

	/**
	* @brief Updates the uniform buffer for a specific frame.
	* 
	* @param ubo The UniformBufferObject from which to copy to the uniform buffer
	* @param currentFrame The index of the frame
	*/
	void update(UniformBufferObject& ubo, uint32_t currentFrame);

	// getters
	vk::Buffer getBuffer(uint32_t currentFrame) const { return buffers[currentFrame]->getBuffer(); }

private:
	Device& dev;
	std::vector<std::unique_ptr<Buffer>> buffers;
};

/* End of the UniformBuffer.hpp file */
