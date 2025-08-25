#include "Device.hpp"
#include <stdexcept>
#include <iostream>
#include <vector>

Device::Device(vk::Instance instance, vk::SurfaceKHR surface) {
	pickPhysicalDevice(instance, surface);
	createLogicalDevice(surface);
}

void Device::pickPhysicalDevice(vk::Instance instance, vk::SurfaceKHR surface) {
	auto availableDevices = instance.enumeratePhysicalDevices();
	auto suitableDevices = std::vector<vk::PhysicalDevice>();

	if (availableDevices.empty()) {
		throw std::runtime_error("No Vulkan-compatible GPU found.");
	}

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

		if (indices.isComplete() && swapchainSupport)
		{
			suitableDevices.push_back(dev);

			graphicsQueueFamily = indices.graphicsFamily.value();
			presentQueueFamily = indices.presentFamily.value();
		}

		vk::PhysicalDeviceProperties props = dev.getProperties();
		std::cout << "Found GPU: " << props.deviceName
			<< " (Suitable: " << (indices.isComplete() && swapchainSupport ? "YES" : "NO")
			<< ")" << std::endl;
	}

	if (!suitableDevices.empty())
	{
		physicalDevice = suitableDevices[0];

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
	
	queueCreateInfos.push_back(graphicsQueueCreateInfo);

	if (graphicsQueueFamily != presentQueueFamily)
	{
		vk::DeviceQueueCreateInfo presentQueueCreateInfo(
			{},
			presentQueueFamily,
			1,
			&queuePriority
		);
		queueCreateInfos.push_back(presentQueueCreateInfo);
	}

	vk::PhysicalDeviceFeatures deviceFeatures = {};

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

	for (uint32_t i = 0; i < queueFamilies.size(); i++)
	{
		if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			indices.graphicsFamily = i;
		}

		if (device.getSurfaceSupportKHR(i, surface))
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}
	}

	return indices;
}