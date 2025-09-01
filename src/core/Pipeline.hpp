#pragma once
#include <vulkan/vulkan.hpp>
#include "../resources/Model.hpp"
#include <memory>
#include <vector>

class Device;
class RenderPass;

class Pipeline {
public:
	Pipeline(const Device& device, const RenderPass& renderPass, vk::Extent2D swapchainExtent);
	~Pipeline() = default;

	vk::Pipeline get() const { return graphicsPipeline.operator*(); }
	vk::PipelineLayout getLayout() const { return pipelineLayout.operator*(); }

private:
	void createPipeline(const RenderPass& renderPass, vk::Extent2D swapchainExtent);
	vk::UniqueShaderModule createShaderModule(const std::vector<char>& inputCode);

	const Device& pipelineDevice;
	vk::UniquePipelineLayout pipelineLayout;
	vk::UniquePipeline graphicsPipeline;
};