/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Device.hpp
 * @brief Device management for Vulkan application.
 *
 * This file contains the implementation of the Device class,
 * which is responsible for selecting a physical device (GPU)
 * and creating a logical device along with the necessary queues.
 * 
 * The Device class encapsulates the Vulkan physical and logical device,
 * as well as the graphics and presentation queues.
 */

#pragma once
#include <vulkan/vulkan.hpp>
#include <optional>
#include <vk_mem_alloc.h>

class Device {
public:
	/**
	 * @brief Constructs a Device object. 
	 * 
	 * @param instance Vulkan instance.
	 * @param surface Vulkan surface for presentation.
	 */
	Device(vk::Instance instance, vk::SurfaceKHR surface);
	~Device();

	/**
	 * @brief Returns the physical device (GPU) being used.
	 *
	 * @return vk::PhysicalDevice - member variable 'physicalDevice'.
	 */
	vk::PhysicalDevice getPhysicalDevice() const {
		return physicalDevice;
	}

	/**
	 * @brief Returns the logical device.
	 *
	 * @return vk::Device - member variable 'device'.
	 */
	vk::Device operator*() const {
		return *device;
	}

	/**
	 * @brief Finds a supported format from a list of candidates.
	 *
	 * @param candidates List of candidate formats.
	 * @param tiling Desired image tiling (optimal, linear).
	 * @param features Format features (depth stencil attachment).
	 *
	 * @return vk::Format - a supported format that meets the criteria.
	 */
	vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

	/**
	 * @brief Finds a suitable depth format for depth buffering.
	 *
	 * @return vk::Format - a supported depth format that can be used for depth buffering.
	*/
	vk::Format findDepthFormat();

	// getters
	vk::Queue getGraphicsQueue() const { return graphicsQueue; }
	vk::Queue getPresentQueue() const { return presentQueue; }
	uint32_t getGraphicsQueueFamily() const { return graphicsQueueFamily; }
	uint32_t getPresentQueueFamily() const { return presentQueueFamily; }
	VmaAllocator getAllocator() const { return allocator; }

private:
	// structure to hold queue family indices
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		// check if both queue families are set
		bool isComplete() const
		{
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	/**
	 * @brief Selects the first suitable physical device (GPU) that supports required features.
	 * 
	 * @param instance Vulkan instance.
	 * @param surface Vulkan surface for presentation.
	 */
	void pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);

	/**
	 * @brief Finds queue families that support graphics and presentation.
	 * 
	 * @param device Vulkan physical device to evaluate.
	 * @param surface Vulkan surface for presentation.
	 * 
	 * @return QueueFamilyIndices - structure containing indices of graphics and presentation queue families.
	 */
	QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);

	/**
	 * @brief Creates a logical device from the first suitable physical device and retrieves the graphics and presentation queues.
	 * 
	 * @param surface Vulkan surface for presentation.
	 */
	void createLogicalDevice(vk::SurfaceKHR surface);

	vk::PhysicalDevice physicalDevice;
	vk::UniqueDevice device;

	vk::Queue graphicsQueue;
	vk::Queue presentQueue;

	VmaAllocator allocator = nullptr;

	uint32_t graphicsQueueFamily;
	uint32_t presentQueueFamily;
};

/* End of the Device.hpp file */