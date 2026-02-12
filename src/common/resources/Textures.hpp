#pragma once

#include <vulkan/vulkan.hpp>
#include <string.h>
#include "../core/Device.hpp"
#include "../rendering/CommandManager.hpp"

class Texture {
public:
	Texture(Device& device, CommandManager& cmd, std::string& filePath);
	~Texture();

	vk::ImageView getImageView() const { return *imageView; }
	vk::Sampler getSampler() const { return *sampler; }

private:
	void createTextureImage(CommandManager& cmd, std::string& filePath);
	void createTextureImageView();
	void createTextureSampler();

	// TODO: move this to buffer class
	void createBuffer(vk::DeviceSize size,
					  vk::BufferUsageFlags usage,
					  vk::MemoryPropertyFlags flags,
					  vk::UniqueBuffer& buffer,
				      vk::UniqueDeviceMemory& bufferMemory);

	// Vulkan resources
	Device& dev;
	vk::UniqueImage image;
	vk::UniqueDeviceMemory imageMemory;
	vk::UniqueImageView imageView;
	vk::UniqueSampler sampler;

	// texture properties
	int width;
	int height;
	int channels;
	vk::DeviceSize imageSize;
};