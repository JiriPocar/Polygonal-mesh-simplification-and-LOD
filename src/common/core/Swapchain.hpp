/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Swapchain.hpp
 * @brief Swapchain management for Vulkan application.
 *
 * This file contains the implementation of the Swapchain class,
 * which is responsible for creating and managing the Vulkan swapchain,
 * including image views and depth resources.
 */

#pragma once
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <vector>
#include <memory>

class Device;

class Swapchain {
public:
	Swapchain(Device& device, vk::SurfaceKHR surface, bool enableVsync, uint32_t width, uint32_t height);
	~Swapchain();

	/**
	* @brief Acquires the next available image from the swapchain.
	* 
	* @param signalSemaphore Semaphore to signal when the image is available
	* 
	* @return uint32_t index of the acquired image in the swapchain
	*/
	uint32_t acquireNextImage(vk::Semaphore signalSemaphore);

	/**
	* @brief Presents the rendered image to the screen.
	* 
	* @param imageIndex Index of the image to present, obtained from acquireNextImage()
	* @param waitSemaphore Semaphore to wait for before presenting the image
	*/
	void presentImage(uint32_t imageIndex,vk::Semaphore waitSemaphore);

	/**
	* @brief Recreates the swapchain when the window is resized.
	* 
	* @param surface Vulkan surface associated with the swapchain
	* @param width New width of the swapchain images
	* @param height New height of the swapchain images
	*/
	void recreateOnResize(vk::SurfaceKHR surface, uint32_t width, uint32_t height);

	/**
	* @brief Cleans up the swapchain resources.
	*/
	void cleanup();
	
	// getters
	const std::vector<vk::Image>& getImages() const { return images; }
	const std::vector<vk::UniqueImageView>& getImageViews() const { return imageViews; }
	vk::ImageView getDepthImageView() const { return depthImageView.get(); }
	vk::Format getImageFormat() const { return imageFormat; }
	vk::Extent2D getExtent() const { return extent; }
	const vk::UniqueSwapchainKHR& get() const { return swapchain; }

private:
	struct SwapchainSupportDetails {
		vk::SurfaceCapabilitiesKHR capabilities;
		std::vector<vk::SurfaceFormatKHR> formats;
		std::vector<vk::PresentModeKHR> presentModes;
	};

	/**
	* @brief Creates the Vulkan swapchain.
	* 
	* @param surface Vulkan surface associated with the swapchain
	* @param width Width of the swapchain images
	* @param height Height of the swapchain images
	*/
	void createSwapchain(vk::SurfaceKHR surface, uint32_t width, uint32_t height);

	// creates image views for the swapchain images
	void createImageViews();

	// creates depth resources (image, memory allocation, and image view) for depth buffering.
	void createDepthResources();

	Device& m_device;
	vk::UniqueSwapchainKHR swapchain;
	std::vector<vk::Image> images;
	std::vector<vk::UniqueImageView> imageViews;
	vk::Format imageFormat;
	vk::Extent2D extent;
	bool enableVsync;

	vk::Image depthImage = nullptr;
	VmaAllocation depthImageAllocation = nullptr;

	vk::UniqueImageView depthImageView;
	vk::Format depthFormat;
};

/* End of the Swapchain.hpp file */