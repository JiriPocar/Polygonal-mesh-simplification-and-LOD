#include "Buffer.hpp"

Buffer::Buffer(const Device& device, vk::DeviceSize size, vk::BufferUsageFlags usage, VmaMemoryUsage memoryUsage, VmaAllocationCreateFlags flags)
	: dev(device), devSize(size)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = static_cast<VkBufferUsageFlags>(usage);
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo vmaAllocInfo = {};
	vmaAllocInfo.usage = memoryUsage;
	vmaAllocInfo.flags = flags;

	VkBuffer rawBuffer;
	if (vmaCreateBuffer(dev.getAllocator(), &bufferInfo, &vmaAllocInfo, &rawBuffer, &allocation, &allocInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create buffer");
	}

	buffer = rawBuffer;
}

Buffer::~Buffer()
{
	if (buffer != nullptr)
	{
		vmaDestroyBuffer(dev.getAllocator(), buffer, allocation);
	}
}

void Buffer::copyData(const void* data, vk::DeviceSize size)
{
	if (size > devSize)
	{
		throw std::runtime_error("Copy data failed due to exceeding buffer size");
	}

	// if the buffer is already mapped, we can copy directly, otherwise, we need to map it first.
	if (allocInfo.pMappedData != nullptr)
	{
		memcpy(allocInfo.pMappedData, data, static_cast<size_t>(size));
	}
	else // map, copy, then unmap
	{
		void* mappedData;
		vmaMapMemory(dev.getAllocator(), allocation, &mappedData);
		memcpy(mappedData, data, static_cast<size_t>(size));
		vmaUnmapMemory(dev.getAllocator(), allocation);
	}

	// flush the allocation to ensure the data is visible to the GPU
	vmaFlushAllocation(dev.getAllocator(), allocation, 0, size);
}

