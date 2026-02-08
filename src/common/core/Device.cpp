/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Device.cpp
 * @brief Device management for Vulkan application.
 *
 * This file contains the implementation of the Device class,
 * which is responsible for selecting a physical device (GPU)
 * and creating a logical device along with the necessary queues.
 */

#include "Device.hpp"
#include <stdexcept>
#include <iostream>
#include <vector>

Device::Device(vk::Instance instance, vk::SurfaceKHR surface)
{
	pickPhysicalDevice(instance, surface);
	createLogicalDevice(surface);
}

void Device::pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface)
{
	auto availableDevices = instance.enumeratePhysicalDevices();
	auto suitableDevices = std::vector<vk::PhysicalDevice>();

	if (availableDevices.empty()) {
		throw std::runtime_error("No Vulkan-compatible GPU found.");
	}

	// evaluate each device
	for (const auto& dev : availableDevices)
	{
		QueueFamilyIndices indices = findQueueFamilies(dev, surface);

		auto extensions = dev.enumerateDeviceExtensionProperties();
		bool swapchainSupport = false;
		for (const auto& ext : extensions)
		{
			if (strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
			{
				swapchainSupport = true;
				break;
			}
		}

		// check if device is suitable
		if (indices.isComplete() && swapchainSupport)
		{
			suitableDevices.push_back(dev);

			graphicsQueueFamily = indices.graphicsFamily.value();
			presentQueueFamily = indices.presentFamily.value();
		}

		// print device info
		vk::PhysicalDeviceProperties props = dev.getProperties();
		std::cout << "Found GPU: " << props.deviceName
			<< " (Suitable: " << (indices.isComplete() && swapchainSupport ? "YES" : "NO")
			<< ")" << std::endl;
	}

	// select the first suitable device
	if (!suitableDevices.empty())
	{
		physicalDevice = suitableDevices[0];

		// print selected device info
		vk::PhysicalDeviceProperties props = physicalDevice.getProperties();
		std::cout << "Selected GPU: " << props.deviceName << std::endl;
		std::cout << "graphicsQueueFamily: " << graphicsQueueFamily
			<< ", presentQueueFamily: " << presentQueueFamily << std::endl;
		return;
	}

	throw std::runtime_error("No suitable GPU found.");
}

void Device::createLogicalDevice(vk::SurfaceKHR surface)
{
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	float queuePriority = 1.0f;

	vk::DeviceQueueCreateInfo graphicsQueueCreateInfo(
		{},
		graphicsQueueFamily,
		1,
		&queuePriority
	);
	
	// add graphics queue info
	queueCreateInfos.push_back(graphicsQueueCreateInfo);

	if (graphicsQueueFamily != presentQueueFamily)
	{
		vk::DeviceQueueCreateInfo presentQueueCreateInfo(
			{},
			presentQueueFamily,
			1,
			&queuePriority
		);

		// add present queue info
		queueCreateInfos.push_back(presentQueueCreateInfo);
	}

	// specify device features
	vk::PhysicalDeviceFeatures deviceFeatures = {};

	// specify device extensions
	std::vector<const char*> extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	device = physicalDevice.createDeviceUnique(
		vk::DeviceCreateInfo(
			{},
			static_cast<uint32_t>(queueCreateInfos.size()),
			queueCreateInfos.data(),
			0,
			nullptr,
			static_cast<uint32_t>(extensions.size()),
			extensions.data(),
			&deviceFeatures
		)
	);

	graphicsQueue = device->getQueue(graphicsQueueFamily, 0);
	presentQueue = device->getQueue(presentQueueFamily, 0);
}

Device::QueueFamilyIndices Device::findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface)
{
	QueueFamilyIndices indices;
	auto queueFamilies = device.getQueueFamilyProperties();

	// loop through each queue family
	for (uint32_t i = 0; i < queueFamilies.size(); i++)
	{
		// check for graphics support
		if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			indices.graphicsFamily = i;
		}

		// check for presentation support
		if (device.getSurfaceSupportKHR(i, surface))
		{
			indices.presentFamily = i;
		}

		// if both families are found, stop searching and return
		if (indices.isComplete())
		{
			break;
		}
	}

	return indices;
}

uint32_t Device::findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const
{
	// find available memory information on the GPU
	vk::PhysicalDeviceMemoryProperties memProps = physicalDevice.getMemoryProperties();

	for (int i = 0; i < memProps.memoryTypeCount; i++)
	{
		// check if the memory type is suitable
		bool typeSupported = typeFilter & (1 << i);

		// check if the memory type has the required properties
		bool hasRequiredProps = (memProps.memoryTypes[i].propertyFlags & properties) == properties;

		if (typeSupported && hasRequiredProps)
		{
			return i; // found
		}
	}

	throw std::runtime_error("Failed to find suitable memory type.");
}

vk::Format Device::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
	// check each candidate format for support
	for (vk::Format format : candidates)
	{
		vk::FormatProperties properties = physicalDevice.getFormatProperties(format);

		if (tiling == vk::ImageTiling::eLinear && (properties.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == vk::ImageTiling::eOptimal && (properties.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}

	throw std::runtime_error("Failed to find supported format!");
}

vk::Format Device::findDepthFormat()
{
	return findSupportedFormat({vk::Format::eD32Sfloat,
								vk::Format::eD32SfloatS8Uint,
								vk::Format::eD24UnormS8Uint},
							    vk::ImageTiling::eOptimal,
							    vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

/* End of the Device.cpp file */