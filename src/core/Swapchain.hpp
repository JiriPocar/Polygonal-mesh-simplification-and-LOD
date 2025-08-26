#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>

class Device;

class Swapchain {
public:
	Swapchain(const Device& device, vk::SurfaceKHR surface, uint32_t width, uint32_t height);
	~Swapchain() = default;

	const std::vector<vk::Image>& getImages() const { return images; }
	const std::vector<vk::UniqueImageView>& getImageViews() const { return imageViews; }
	vk::Format getImageFormat() const { return imageFormat; }
	vk::Extent2D getExtent() const { return extent; }

	uint32_t acquireNextImage(vk::Semaphore signalSemaphore);
	void presentImage(uint32_t imageIndex,vk::Semaphore waitSemaphore);

private:
	struct SwapchainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	const Device& swapchainDevice; // Reference to the device
	vk::UniqueSwapchainKHR swapchain; // Unique handle to the swapchain
	std::vector<vk::Image> images; // Images in the swapchain
	std::vector<vk::UniqueImageView> imageViews; // Image views for the swapchain images
	vk::Format imageFormat; // Format of the swapchain images
	vk::Extent2D extent; // Extent (width and height) of the swapchain images

	void createSwapchain(vk::SurfaceKHR surface, uint32_t width, uint32_t height);
	void createImageViews();
};
