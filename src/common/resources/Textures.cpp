/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Textures.hpp
 * @brief Texture management for Vulkan application.
 *
 * This file implements the loading an image from a file, creating a Vulkan image, allocating
 * memory for it using VMA and creating an image view and sampler.
 * 
 * Parts of the code may be inspired or adapted from:
 *		- Alexander Overvoorde's "Vulkan Tutorial"
 *			- @url https://vulkan-tutorial.com/
 *			- @url https://github.com/Overv/VulkanTutorial
 *		- VMA's "VulkanSample"
 *			- @url https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/src/VulkanSample.cpp
 */

#include "Textures.hpp"

#include "stb_image.h"
#include "common/resources/Buffer.hpp"
#include <stdexcept>
#include <iostream>

Texture::Texture(Device& device, CommandManager& cmd, std::string& filePath)
	: dev(device)
{
	createTextureImage(cmd, filePath);
	createTextureImageView();
	createTextureSampler();
}

Texture::~Texture()
{
	if (image != nullptr)
	{
		vmaDestroyImage(dev.getAllocator(), image, imageAllocation);
	}
}

void Texture::createTextureImage(CommandManager& cmd, std::string& filePath)
{
	// load image data using stb_image
	stbi_uc* pixels = stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("Failed to load texture image!");
	}

	imageSize = width * height * 4; // 4 bytes per pixel (RGBA)

	// create staging buffer and copy pixel data to it
	Buffer stagingBuffer(
		dev,
		imageSize,
		vk::BufferUsageFlagBits::eTransferSrc,
		VMA_MEMORY_USAGE_CPU_ONLY,
		VMA_ALLOCATION_CREATE_MAPPED_BIT
	);
	stagingBuffer.copyData(pixels, imageSize);
	stbi_image_free(pixels);

	// create optimal tiled image on GPU
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	// vmaCreateImage can create the image and allocate memory for it in one call
	VkImage rawImage;
	auto result = vmaCreateImage(dev.getAllocator(), &imageInfo, &allocInfo, &rawImage, &imageAllocation, nullptr);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create texture image!");
	}
	image = rawImage;

	// transition to transfer destination layout for copy
	cmd.transitionImageLayout(
		image,
		vk::Format::eR8G8B8A8Srgb,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal
	);

	// copy buffer to image
	cmd.copyBufferToImage(
		stagingBuffer.getBuffer(),
		image,
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	);

	// transition to shader read layout for sampling in shader
	cmd.transitionImageLayout(
		image,
		vk::Format::eR8G8B8A8Srgb,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal
	);
}

void Texture::createTextureImageView()
{
	vk::ImageViewCreateInfo viewInfo(
		{},
		image,
		vk::ImageViewType::e2D,
		vk::Format::eR8G8B8A8Srgb,
		{},
		vk::ImageSubresourceRange(
			vk::ImageAspectFlagBits::eColor,
			0, 1, 0, 1
		)
	);

	imageView = dev.operator*().createImageViewUnique(viewInfo);
}

void Texture::createTextureSampler()
{
	vk::PhysicalDeviceProperties props = dev.getPhysicalDevice().getProperties();

	vk::SamplerCreateInfo samplerInfo(
		{},										// flags
		vk::Filter::eLinear,					// magnification (Linear = blurred, Nearest = pixelated)
		vk::Filter::eLinear,					// minification	
		vk::SamplerMipmapMode::eLinear,			// mipmapMode
		vk::SamplerAddressMode::eRepeat,		// when out of bounds, repeat the texture
		vk::SamplerAddressMode::eRepeat,
		vk::SamplerAddressMode::eRepeat,
		0.0f,									// mipLodBias	
		VK_TRUE,								// anisotropyEnable
		props.limits.maxSamplerAnisotropy,		// maxAnisotropy
		VK_FALSE,								// compareEnable
		vk::CompareOp::eAlways,					// compareOp
		0.0f,									// minLod	
		0.0f,									// maxLod	
		vk::BorderColor::eIntOpaqueBlack,		// borderColor
		VK_FALSE								// unnormalizedCoordinates	
	);

	sampler = dev.operator*().createSamplerUnique(samplerInfo);
}

/* End of the Textures.cpp file */