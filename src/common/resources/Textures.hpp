/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Textures.hpp
 * @brief Texture management for Vulkan application.
 *
 * This file implements the Texture class, which is responsible
 * for loading an image from a file, creating a Vulkan image, allocating
 * memory for it using VMA, and creating an image view and sampler.
 */

#pragma once
#include <vulkan/vulkan.hpp>
#include <string.h>
#include <vk_mem_alloc.h>
#include "common/core/Device.hpp"
#include "common/rendering/CommandManager.hpp"

class Texture {
public:
	Texture(Device& device, CommandManager& cmd, std::string& filePath);
	~Texture();

	// getters
	vk::ImageView getImageView() const { return *imageView; }
	vk::Sampler getSampler() const { return *sampler; }

private:
	// helper functions for texture creation
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

/* End of the Textures.hpp file */