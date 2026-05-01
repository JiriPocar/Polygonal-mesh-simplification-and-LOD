/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Buffer.hpp
 * @brief Buffer management for Vulkan application.
 *
 * This file implements the Buffer class, creating and managing Vulkan
 * buffers using VMA for memory management.
 */

#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "common/core/Device.hpp"

class Buffer {
public:
	Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags = 0);
	~Buffer();

	// prevent copying, RAII handles resource management
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	// getters
	vk::Buffer getBuffer() const { return buffer; }
	vk::DeviceSize getSize() const { return devSize; }
	void* getMappedData() const { return allocInfo.pMappedData; }

	/**
	* @brief Copies data to the buffer.
	* 
	* @param data Pointer to the data to copy to the buffer
	* @param size Size of the data to copy (bytes)
	*/
	void copyData(const void* data, vk::DeviceSize size);

private:
	const Device& dev;
	vk::Buffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocInfo = {};
	vk::DeviceSize devSize;
};

/* End of the Buffer.hpp file */