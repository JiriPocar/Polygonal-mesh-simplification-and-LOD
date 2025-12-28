#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>

class Device;

class RenderPass {
public:
	RenderPass(const Device& device, vk::Format swapchainImageFormat);
	~RenderPass() = default;

	vk::RenderPass get() const { return renderPass.operator*(); }
	vk::Format getFormat() const { return format; }

private:
	void createRenderpass(vk::Format swapchainImageFormat);

	vk::Format format;
	vk::UniqueRenderPass renderPass;
	const Device& renderPassDevice;
};