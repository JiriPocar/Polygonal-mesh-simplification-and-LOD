#pragma once

#include <vulkan/vulkan.hpp>
#include "../core/Device.hpp"

class Buffer {
public:
	Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
	~Buffer() = default;

	vk::Buffer getBuffer() const { return *buffer; }
	vk::DeviceMemory getMemory() const { return *memory; }
	vk::DeviceSize getSize() const { return devSize; }

	void* map();
	void unmap();
	void copyData(const void* data, vk::DeviceSize size);

private:
	const Device& dev;
	vk::UniqueBuffer buffer;
	vk::UniqueDeviceMemory memory;
	vk::DeviceSize devSize;
};