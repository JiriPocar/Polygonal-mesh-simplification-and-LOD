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
	Pipeline(Device& device,
		RenderPass& renderPass,
		vk::Extent2D swapchainExtent,
		vk::DescriptorSetLayout descriptorSetLayout,
		vk::PolygonMode polygonMode = vk::PolygonMode::eFill);
	~Pipeline() = default;

	vk::Pipeline get() const { return graphicsPipeline.operator*(); }
	vk::PipelineLayout getLayout() const { return pipelineLayout.operator*(); }
	vk::DescriptorSetLayout getDescriptorSetLayout() const { return *descriptorSetLayout; }

private:
	void createPipeline(RenderPass& renderPass, vk::Extent2D swapchainExtent, vk::DescriptorSetLayout descriptorSetLayout, vk::PolygonMode polygonMode);
	vk::UniqueShaderModule createShaderModule(const std::vector<char>& inputCode);

	Device& pipelineDevice;
	vk::UniquePipelineLayout pipelineLayout;
	vk::UniquePipeline graphicsPipeline;
	vk::UniqueDescriptorSetLayout descriptorSetLayout;
};

/* End of the Pipeline.hpp file */