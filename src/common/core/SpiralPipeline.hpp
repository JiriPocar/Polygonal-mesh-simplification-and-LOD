#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>
#include <vector>
#include <string>

#include "../resources/Model.hpp"
#include "../rendering/RenderPass.hpp"
#include "../core/Device.hpp"

class SpiralPipeline {
public:
	SpiralPipeline(Device& device,
				   RenderPass& renderPass,
		           vk::Extent2D swapchainExtent,
		           vk::DescriptorSetLayout descSetLayout);
	~SpiralPipeline() = default;

	vk::Pipeline get() const { return graphicsPipeline.get(); }
	vk::PipelineLayout getLayout() const { return pipelineLayout.get(); }
private:
	void createPipeline(const RenderPass& renderPass,
		vk::Extent2D swapchainExtent,
		vk::DescriptorSetLayout descSetLayout);
	vk::UniqueShaderModule createShaderModule(const std::vector<char>& code);

	Device& pipelineDevice;
	vk::UniquePipelineLayout pipelineLayout;
	vk::UniquePipeline graphicsPipeline;
};