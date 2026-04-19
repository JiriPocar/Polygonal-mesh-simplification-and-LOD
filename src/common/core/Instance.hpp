/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Instance.hpp
 * @brief Instance creation for Vulkan application with extension and validation layer support.
 *
 * This file contains the implementation of the Instance class,
 * which is responsible for creating an unique Vulkan instance.
 */

#pragma once
#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include <vulkan/vulkan.hpp>
#include <memory>

class Instance
{
public:
	/**
	 * @brief Constructs a Vulkan instance with optional validation layers.
	 * 
	 * @param validation If true, enables validation layers for debugging
	 */
	Instance(bool validation);
	~Instance() = default;

	// getters
	operator vk::Instance() const { return *instance; }
	vk::Instance get() const { return *instance; }

private:
	vk::UniqueInstance instance;
};

/* End of the Instance.hpp file */