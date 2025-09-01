#pragma once
#include <vulkan/vulkan.hpp>
#include <optional>

class Device {
public:
	Device(vk::Instance instance, vk::SurfaceKHR surface);
	~Device() = default;

	vk::PhysicalDevice getPhysicalDevice() const {
		return physicalDevice;
	}

	vk::Device operator*() const {
		return *device;
	}

	vk::Queue getGraphicsQueue() const {
		return graphicsQueue;
	}

	vk::Queue getPresentQueue() const {
		return presentQueue;
	}

	uint32_t getGraphicsQueueFamily() const {
		return 0;
	}

	uint32_t getPresentQueueFamily() const {
		return 0;
	}



private:
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() const {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	void pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface);
	QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);
	void createLogicalDevice(vk::SurfaceKHR surface);

	vk::PhysicalDevice physicalDevice;
	vk::UniqueDevice device;

	vk::Queue graphicsQueue;
	vk::Queue presentQueue;

	uint32_t graphicsQueueFamily;
	uint32_t presentQueueFamily;
};