#pragma once
#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XCB_KHR
#endif
#include <vulkan/vulkan.hpp>
#include <memory> // unique_ptr etc.

class Instance
{
public:
	Instance(bool validation);
	~Instance() = default;

	operator vk::Instance() const
	{
		return *instance;
	}

private:
	vk::UniqueInstance instance;
};