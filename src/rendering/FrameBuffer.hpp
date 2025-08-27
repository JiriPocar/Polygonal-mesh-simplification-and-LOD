#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>
#include <memory>

class Device;
class Swapchain;
class RenderPass;

class FrameBuffer {
public:
	FrameBuffer(const Device& device, const RenderPass& renderPass, const Swapchain& swapchain);
	~FrameBuffer() = default;

	const std::vector<vk::Framebuffer>& getFramebuffers() const {
		// Convert vector of vk::UniqueFramebuffer to vector of vk::Framebuffer
		static std::vector<vk::Framebuffer> fbHandles;
		fbHandles.clear();
		for (const auto& fb : framebuffers) {
			fbHandles.push_back(fb.get());
		}
		return fbHandles;
	}
	vk::Framebuffer getFrameBufferAt(size_t index) const { return framebuffers.at(index).get(); }

private:
	void createFramebuffers(const RenderPass& renderPass, const Swapchain& swapchain);

	const Device& dev;
	std::vector<vk::UniqueFramebuffer> framebuffers;
};