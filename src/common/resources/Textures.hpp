#pragma once

#include <vulkan/vulkan.hpp>
#include <string.h>
#include <vk_mem_alloc.h>
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

	// Vulkan resources
	Device& dev;
	vk::Image image = nullptr;
	VmaAllocation imageAllocation = nullptr;
	vk::UniqueImageView imageView;
	vk::UniqueSampler sampler;

	// texture properties
	int width;
	int height;
	int channels;
	vk::DeviceSize imageSize;
};