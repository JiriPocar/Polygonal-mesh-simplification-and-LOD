/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Pipeline.cpp
 * @brief Pipeline class declaration for Vulkan application.
 *
 * This file contains the declaration of the Pipeline class.
 */

#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>
#include <vector>

#include "../resources/Model.hpp"
#include "Device.hpp"
#include "../rendering/RenderPass.hpp"

class Pipeline {
public:
	Pipeline(const Device& device, const RenderPass& renderPass, vk::Extent2D swapchainExtent, vk::DescriptorSetLayout descriptorSetLayout);
	~Pipeline() = default;

	vk::Pipeline get() const { return graphicsPipeline.operator*(); }
	vk::PipelineLayout getLayout() const { return pipelineLayout.operator*(); }
	vk::DescriptorSetLayout getDescriptorSetLayout() const { return *descriptorSetLayout; }

private:
	void createPipeline(const RenderPass& renderPass, vk::Extent2D swapchainExtent, vk::DescriptorSetLayout descriptorSetLayout);
	vk::UniqueShaderModule createShaderModule(const std::vector<char>& inputCode);

	const Device& pipelineDevice;
	vk::UniquePipelineLayout pipelineLayout;
	vk::UniquePipeline graphicsPipeline;
	vk::UniqueDescriptorSetLayout descriptorSetLayout;
};

/* End of the Pipeline.hpp file */