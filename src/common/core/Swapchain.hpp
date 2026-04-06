#pragma once
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <vector>
#include <memory>

class Device;

class Swapchain {
public:
	Swapchain(Device& device, vk::SurfaceKHR surface, uint32_t width, uint32_t height);
	~Swapchain();

	const std::vector<vk::Image>& getImages() const { return images; }
	const std::vector<vk::UniqueImageView>& getImageViews() const { return imageViews; }
	vk::ImageView getDepthImageView() const { return depthImageView.get(); }
	vk::Format getImageFormat() const { return imageFormat; }
	vk::Extent2D getExtent() const { return extent; }
	const vk::UniqueSwapchainKHR& get() const { return swapchain; }

	uint32_t acquireNextImage(vk::Semaphore signalSemaphore);
	void presentImage(uint32_t imageIndex,vk::Semaphore waitSemaphore);

	void recreateOnResize(vk::SurfaceKHR surface, uint32_t width, uint32_t height);
	void cleanup();

private:
	struct SwapchainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	Device& swapchainDevice;
	vk::UniqueSwapchainKHR swapchain;
	std::vector<vk::Image> images;
	std::vector<vk::UniqueImageView> imageViews;
	vk::Format imageFormat;
	vk::Extent2D extent;

	vk::Image depthImage = nullptr;
	VmaAllocation depthImageAllocation = nullptr;

	vk::UniqueImageView depthImageView;
	vk::Format depthFormat;

	void createSwapchain(vk::SurfaceKHR surface, uint32_t width, uint32_t height);
	void createImageViews();
	void createDepthResources();
};
