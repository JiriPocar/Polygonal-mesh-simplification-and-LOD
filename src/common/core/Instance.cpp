/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Instance.cpp
 * @brief Instance creation for Vulkan application with extension and validation layer support.
 *
 * This file contains the implementation of the Instance class,
 * which is responsible for creating an unique Vulkan instance.
 * 
 * Parts of the code may be inspired or adapted from:
 *		- Alexander Overvoorde's "Vulkan Tutorial"
 *			- @url https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Instance
 *			- @url https://github.com/Overv/VulkanTutorial
 */

#include "Instance.hpp"
#include <vector>

Instance::Instance(bool validation)
{
	std::vector<const char*> extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		#ifdef _WIN32
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		#elif defined(__linux__)
		VK_KHR_XCB_SURFACE_EXTENSION_NAME,
		#endif
	};

	std::vector<const char*> layers;
	if (validation)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		layers.push_back("VK_LAYER_KHRONOS_validation");
	}

	vk::ApplicationInfo appInfo = {
		"Level of Detail techniques",	// application name
		VK_MAKE_VERSION(0,0,0), // application version
		nullptr,				// engine name
		VK_MAKE_VERSION(0,0,0), // engine version
		VK_API_VERSION_1_3		// api version
	};

	vk::InstanceCreateInfo createInfo = {
		{},											// flags
		&appInfo,								    // application info	
		static_cast<uint32_t>(layers.size()),		// number of enabled layers
		layers.data(),						        // enabled layers
		static_cast<uint32_t>(extensions.size()),   // number of enabled extensions
		extensions.data()						    // enabled extensions
	};

	instance = vk::createInstanceUnique(createInfo);
}

/* End of the Instance.cpp file */