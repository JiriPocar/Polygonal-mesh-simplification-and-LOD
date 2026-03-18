#pragma once

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include "../core/Device.hpp"

class Buffer {
public:
	Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags = 0);
	~Buffer();

	// prevent copying, RAII handles resource management
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	vk::Buffer getBuffer() const { return buffer; }
	vk::DeviceSize getSize() const { return devSize; }

	void* getMappedData() const { return allocInfo.pMappedData; }
	void copyData(const void* data, vk::DeviceSize size);

private:
	const Device& dev;
	vk::Buffer buffer = nullptr;
	VmaAllocation allocation = nullptr;
	VmaAllocationInfo allocInfo = {};
	vk::DeviceSize devSize;
};