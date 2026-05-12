/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Swapchain.hpp
 * @brief Swapchain management for Vulkan application.
 *
 * This file contains the implementation of the Swapchain class,
 * which is responsible for creating and managing the Vulkan swapchain,
 * including image views and depth resources.
 *
 * Parts of the code may be inspired or adapted from:
 *		- Alexander Overvoorde's "Vulkan Tutorial"
 *			- @url https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Swap_chain
 *			- @url https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Image_views
 *			- @url https://vulkan-tutorial.com/Depth_buffering
 *			- @url https://github.com/Overv/VulkanTutorial
 */

#include "Swapchain.hpp"
#include "Device.hpp"
#include <stdexcept>
#include <iostream>

Swapchain::Swapchain(Device& device, vk::SurfaceKHR surface, bool enableVsync, uint32_t width, uint32_t height)
	: m_device(device), enableVsync(enableVsync)
{
	createSwapchain(surface, width, height);
	createImageViews();
	createDepthResources();
}

Swapchain::~Swapchain()
{
	cleanup();
}

void Swapchain::createSwapchain(vk::SurfaceKHR surface, uint32_t width, uint32_t height)
{
	vk::PhysicalDevice physicalDevice = m_device.getPhysicalDevice();
	
	// get swapchain support
	SwapchainSupportDetails supportDetails = {
		physicalDevice.getSurfaceCapabilitiesKHR(surface),
		physicalDevice.getSurfaceFormatsKHR(surface),
		physicalDevice.getSurfacePresentModesKHR(surface)
	};

	// pick the best surface format
	imageFormat = supportDetails.formats[0].format;
	for (const auto& availableFormat : supportDetails.formats) {
		if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			imageFormat = availableFormat.format;
			break;
		}
	}
	
	// pick vsync mode by default, fallback to mailbox or immediate if vsync is disabled
	vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
	if (!enableVsync)
	{
		for (const auto& availablePresentMode : supportDetails.presentModes)
		{
			/*if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
				std::cout << "Mailbox present mode supported, using it for swapchain." << std::endl;
				presentMode = availablePresentMode;
				break;
			}*/
			if (availablePresentMode == vk::PresentModeKHR::eImmediate)
			{
				std::cout << "Immediate present mode supported, using it for swapchain." << std::endl;
				presentMode = availablePresentMode;
				break;
			}
		}
	}

	// determine the image count
	uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;
	if(supportDetails.capabilities.minImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount)
	{
		imageCount = supportDetails.capabilities.maxImageCount;
	}

	// set extent
	extent = vk::Extent2D{width, height};
	if (supportDetails.capabilities.currentExtent.width != UINT32_MAX)
	{
		extent = supportDetails.capabilities.currentExtent;
	}

	// create the swapchain
	vk::SwapchainCreateInfoKHR createInfo(
		{},											// flags
		surface,									// surface
		imageCount,									// minImageCount		
		imageFormat,								// imageFormat
		vk::ColorSpaceKHR::eSrgbNonlinear,			// imageColorSpace
		extent,										// imageExtent		
		1,											// imageArrayLayers
		vk::ImageUsageFlagBits::eColorAttachment,	// imageUsage
		vk::SharingMode::eExclusive,				// imageSharingMode
		0, nullptr,									// queueFamilyIndices
		supportDetails.capabilities.currentTransform, // preTransform
		vk::CompositeAlphaFlagBitsKHR::eOpaque,		// compositeAlpha
		presentMode,								// presentMode
		VK_TRUE										// clipped
	);

	swapchain = m_device.operator*().createSwapchainKHRUnique(createInfo);
	images = m_device.operator*().getSwapchainImagesKHR(*swapchain);
}

void Swapchain::createImageViews()
{
	imageViews.reserve(images.size());

	for (const auto& image : images)
	{
		vk::ImageViewCreateInfo createInfo(
			{},
			image,
			vk::ImageViewType::e2D,
			imageFormat,
			vk::ComponentMapping{
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity,
				vk::ComponentSwizzle::eIdentity
			},
			vk::ImageSubresourceRange{
				vk::ImageAspectFlagBits::eColor,
				0, 1, 0, 1 // mip levels
			}
		);

		imageViews.push_back(m_device.operator*().createImageViewUnique(createInfo));
	}
}

void Swapchain::createDepthResources()
{
	// find a supported depth format
	depthFormat = m_device.findDepthFormat();

	// create depth image using VMA
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = static_cast<VkFormat>(depthFormat);
	imageInfo.extent = { extent.width, extent.height, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// allocate memory for the depth image with VMA
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	VkImage rawImage;
	if (vmaCreateImage(m_device.getAllocator(), &imageInfo, &allocInfo, &rawImage, &depthImageAllocation, nullptr) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create depth image with VMA...");
	}

	depthImage = rawImage;

	// create image view for the depth image
	vk::ImageViewCreateInfo viewInfo(
		{},										// flags
		depthImage,								// image
		vk::ImageViewType::e2D,					// viewType
		depthFormat,							// format
		{},										// components
		vk::ImageSubresourceRange{				// subresourceRange
			vk::ImageAspectFlagBits::eDepth,
			0, 1, 0, 1
		}
	);
	depthImageView = m_device.operator*().createImageViewUnique(viewInfo);
}

uint32_t Swapchain::acquireNextImage(vk::Semaphore signalSemaphore)
{
	auto result = m_device.operator*().acquireNextImageKHR(*swapchain, UINT64_MAX, signalSemaphore, nullptr);
	
	if (result.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to acquire swapchain image.");
	}

	return result.value;
}

void Swapchain::presentImage(uint32_t imageIndex, vk::Semaphore waitSemaphore)
{
	vk::PresentInfoKHR presentInfo(
		1, &waitSemaphore,
		1, &(*swapchain),
		&imageIndex,
		nullptr // pResults
	);

	auto result = m_device.getPresentQueue().presentKHR(presentInfo);
	if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to present swapchain image.");
	}
}

void Swapchain::recreateOnResize(vk::SurfaceKHR surface, uint32_t width, uint32_t height)
{
	// wait for the device to be idle before recreating
	m_device.operator*().waitIdle();

	// cleanup old swapchain
	cleanup();

	// create new swapchain
	createSwapchain(surface, width, height);
	createImageViews();
	createDepthResources();
}

void Swapchain::cleanup()
{
	imageViews.clear();
	swapchain.reset();
	images.clear();
	depthImageView.reset();

	if (depthImage)
	{
		vmaDestroyImage(m_device.getAllocator(), depthImage, depthImageAllocation);
		depthImage = VK_NULL_HANDLE;
		depthImageAllocation = VK_NULL_HANDLE;
	}
}

/* End of the Swapchain.cpp file */