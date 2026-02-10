#include "Textures.hpp"

#include "../external/tinygltf/stb_image.h"

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
}

void Texture::createTextureImage(CommandManager& cmd, std::string& filePath)
{
	// load image data using stb_image
	stbi_uc* pixels = stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	if (!pixels) {
		throw std::runtime_error("Failed to load texture image!");
	}

	imageSize = width * height * 4; // 4 bytes per pixel (RGBA)

	// create staging buffer for image data
	vk::UniqueBuffer stagingBuffer;
	vk::UniqueDeviceMemory stagingBufferMemory;
	createBuffer(
		imageSize,
		vk::BufferUsageFlagBits::eTransferSrc, // src for copying to image
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, // cpu can write to this buffer
		stagingBuffer,
		stagingBufferMemory
	);

	// copy pixel data to staging buffer
	void* data = dev.operator*().mapMemory(*stagingBufferMemory, 0, imageSize);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	dev.operator*().unmapMemory(*stagingBufferMemory);

	stbi_image_free(pixels);

	// create optimal tiled image for the texture
	vk::ImageCreateInfo imageInfo(
		{},									// flags
		vk::ImageType::e2D,					// image type
		vk::Format::eR8G8B8A8Srgb,			// format (srgb, 8 bits per channel)
		vk::Extent3D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
		1,
		1,
		vk::SampleCountFlagBits::e1,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled
	);

	image = dev.operator*().createImageUnique(imageInfo);

	vk::MemoryRequirements memRequirements = dev.operator*().getImageMemoryRequirements(*image);
	vk::MemoryAllocateInfo allocateInfo(
		memRequirements.size,
		dev.findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
	);

	imageMemory = dev.operator*().allocateMemoryUnique(allocateInfo);
	dev.operator*().bindImageMemory(*image, *imageMemory, 0);

	// transition image layout
	cmd.transitionImageLayout(
		*image,
		vk::Format::eR8G8B8A8Srgb,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal
	);

	// copy data from staging buffer to image
	cmd.copyBufferToImage(
		*stagingBuffer,
		*image,
		static_cast<uint32_t>(width),
		static_cast<uint32_t>(height)
	);

	// transition image layout for shader access
	cmd.transitionImageLayout(
		*image,
		vk::Format::eR8G8B8A8Srgb,
		vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal
	);
}

void Texture::createTextureImageView()
{
	vk::ImageViewCreateInfo viewInfo(
		{},
		*image,
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

void Texture::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
						   vk::MemoryPropertyFlags flags, vk::UniqueBuffer& buffer,
						   vk::UniqueDeviceMemory& bufferMemory)
{
	vk::BufferCreateInfo bufferInfo(
		{},
		size,
		usage,
		vk::SharingMode::eExclusive
	);
	
	buffer = dev.operator*().createBufferUnique(bufferInfo);
	vk::MemoryRequirements memRequirements = dev.operator*().getBufferMemoryRequirements(*buffer);
	vk::MemoryAllocateInfo allocInfo(
		memRequirements.size,
		dev.findMemoryType(memRequirements.memoryTypeBits, flags)
	);

	bufferMemory = dev.operator*().allocateMemoryUnique(allocInfo);
	dev.operator*().bindBufferMemory(*buffer, *bufferMemory, 0);
}