#include "Buffer.hpp"

Buffer::Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
	: dev(device), devSize(size)
{
	vk::BufferCreateInfo bufferInfo(
		{},
		size,
		usage,
		vk::SharingMode::eExclusive
	);

	buffer = dev.operator*().createBufferUnique(bufferInfo);
	vk::MemoryRequirements memRequirements = dev.operator*().getBufferMemoryRequirements(*buffer);
	vk::PhysicalDeviceMemoryProperties memProperties = dev.getPhysicalDevice().getMemoryProperties();

	// find suitable memory type
	uint32_t memTypeIndex = 0;
	bool found = false;
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((memRequirements.memoryTypeBits & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			memTypeIndex = i;
			found = true;
			break;
		}
	}

	// throw error if no suitable memory type found
	if (!found) {
		throw std::runtime_error("Failed to find suitable memory type for buffer");
	}

	vk::MemoryAllocateInfo allocInfo(
		memRequirements.size,
		memTypeIndex
	);

	// allocate memory
	memory = dev.operator*().allocateMemoryUnique(allocInfo);
	// bind memory to buffer
	dev.operator*().bindBufferMemory(*buffer, *memory, 0);
}

void* Buffer::map() {
	return dev.operator*().mapMemory(*memory, 0, devSize);
}

void Buffer::unmap() {
	dev.operator*().unmapMemory(*memory);
}

void Buffer::copyData(const void* data, vk::DeviceSize size) {
	if (size > devSize) {
		throw std::runtime_error("Data size exceeds buffer size");
	}

	void* mappedData = map();
	memcpy(mappedData, data, static_cast<size_t>(size));
	unmap();
}

