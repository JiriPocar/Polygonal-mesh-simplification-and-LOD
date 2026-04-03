#include "Swapchain.hpp"
#include "Device.hpp"
#include <stdexcept>
#include <iostream>

Swapchain::Swapchain(Device& device, vk::SurfaceKHR surface, uint32_t width, uint32_t height)
	: swapchainDevice(device) // initialize the swapchain "device" variable with the provided device
{
	createSwapchain(surface, width, height);
	createImageViews();
	createDepthResources();
}

void Swapchain::createSwapchain(vk::SurfaceKHR surface, uint32_t width, uint32_t height)
{
	vk::PhysicalDevice physicalDevice = swapchainDevice.getPhysicalDevice();
	
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
	
	// pick the best present mode
	vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
	for (const auto& availablePresentMode : supportDetails.presentModes) {
		if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
			presentMode = availablePresentMode;
			break;
		}
		//if (availablePresentMode == vk::PresentModeKHR::eImmediate)
		//{
		//	presentMode = availablePresentMode;
		//	break;
		//}
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
		vk::SurfaceTransformFlagBitsKHR::eIdentity, // preTransform
		vk::CompositeAlphaFlagBitsKHR::eOpaque,		// compositeAlpha
		presentMode,								// presentMode
		VK_TRUE										// clipped
	);

	swapchain = swapchainDevice.operator*().createSwapchainKHRUnique(createInfo);
	images = swapchainDevice.operator*().getSwapchainImagesKHR(*swapchain);

	std::cout << "Swapchain created with " << images.size() << " images, format: "
		<< vk::to_string(imageFormat) << ", extent: " << extent.width << "x" << extent.height << std::endl;
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

		imageViews.push_back(swapchainDevice.operator*().createImageViewUnique(createInfo));
	}

	std::cout << "Created " << imageViews.size() << " image views for the swapchain images." << std::endl;
}

void Swapchain::createDepthResources()
{
	// find a supported depth format
	depthFormat = swapchainDevice.findDepthFormat();

	// create depth image
	vk::ImageCreateInfo imageInfo(
		{},													// flags
		vk::ImageType::e2D,									// imageType
		depthFormat,										// format
		vk::Extent3D{ extent.width, extent.height, 1 },		// extent
		1,													// mipLevels
		1,													// arrayLayers
		vk::SampleCountFlagBits::e1,						// samples
		vk::ImageTiling::eOptimal,							// tiling
		vk::ImageUsageFlagBits::eDepthStencilAttachment 	// usage
	);
	depthImage = swapchainDevice.operator*().createImageUnique(imageInfo);

	// allocate memory for the depth image
	vk::MemoryRequirements memRequirements = swapchainDevice.operator*().getImageMemoryRequirements(*depthImage);
	vk::MemoryAllocateInfo allocInfo(
		memRequirements.size,
		swapchainDevice.findMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
	);
	depthImageMemory = swapchainDevice.operator*().allocateMemoryUnique(allocInfo);
	swapchainDevice.operator*().bindImageMemory(*depthImage, *depthImageMemory, 0);

	// create image view for the depth image
	vk::ImageViewCreateInfo viewInfo(
		{},										// flags
		*depthImage,							// image
		vk::ImageViewType::e2D,					// viewType
		depthFormat,							// format
		{},										// components
		vk::ImageSubresourceRange{				// subresourceRange
			vk::ImageAspectFlagBits::eDepth,
			0, 1, 0, 1
		}
	);
	depthImageView = swapchainDevice.operator*().createImageViewUnique(viewInfo);
}

uint32_t Swapchain::acquireNextImage(vk::Semaphore signalSemaphore)
{
	auto result = swapchainDevice.operator*().acquireNextImageKHR(*swapchain, UINT64_MAX, signalSemaphore, nullptr);
	
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

	auto result = swapchainDevice.getPresentQueue().presentKHR(presentInfo);
	if (result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to present swapchain image.");
	}
}

void Swapchain::recreateOnResize(vk::SurfaceKHR surface, uint32_t width, uint32_t height)
{
	// wait for the device to be idle before recreating
	swapchainDevice.operator*().waitIdle();

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
	depthImage.reset();
	depthImageMemory.reset();
}