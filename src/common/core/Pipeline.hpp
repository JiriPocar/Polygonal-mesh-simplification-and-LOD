/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Pipeline.hpp
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

enum class PipelineType {
	Simplificator,
	Spiral
};

class Pipeline {
public:
	Pipeline(Device& device,
		RenderPass& renderPass,
		vk::Extent2D swapchainExtent,
		vk::DescriptorSetLayout descriptorSetLayout,
		PipelineType type,
		vk::PolygonMode polygonMode = vk::PolygonMode::eFill);
	~Pipeline() = default;

	vk::Pipeline get() const { return graphicsPipeline.operator*(); }
	vk::PipelineLayout getLayout() const { return pipelineLayout.operator*(); }

private:
	void createPipeline(RenderPass& renderPass, vk::Extent2D swapchainExtent, vk::DescriptorSetLayout descriptorSetLayout, PipelineType type, vk::PolygonMode polygonMode);
	vk::UniqueShaderModule createShaderModule(const std::vector<char>& inputCode);

	Device& pipelineDevice;
	vk::UniquePipelineLayout pipelineLayout;
	vk::UniquePipeline graphicsPipeline;
};

/* End of the Pipeline.hpp file */