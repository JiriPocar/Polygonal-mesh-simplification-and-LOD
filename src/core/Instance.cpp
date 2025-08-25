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
		"LOD in huge scenes",
		VK_MAKE_VERSION(0,0,0),
		nullptr,
		VK_MAKE_VERSION(0,0,0),
		VK_API_VERSION_1_3
	};

	vk::InstanceCreateInfo createInfo = {
		{},
		&appInfo,
		static_cast<uint32_t>(layers.size()),
		layers.data(),
		static_cast<uint32_t>(extensions.size()),
		extensions.data()
	};

	instance = vk::createInstanceUnique(createInfo);
}