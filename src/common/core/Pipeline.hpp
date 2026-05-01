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

#include "common/resources/Model.hpp"
#include "Device.hpp"
#include "common/rendering/RenderPass.hpp"

// differentiate between apps to use different pipelines
// TODO: make this a builder class instead
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

	// getters
	vk::Pipeline get() const { return graphicsPipeline.operator*(); }
	vk::PipelineLayout getLayout() const { return pipelineLayout.operator*(); }

private:
	/**
	* @brief Creates a graphics pipeline for rendering.
	* 
	* @param renderPass Reference to the RenderPass object defining the render pass configuration
	* @param swapchainExtent Extent of the swapchain images (width and height)
	* @param descriptorSetLayout Descriptor set layout used by the pipeline for resource binding
	* @param type Type of the pipeline to create which determines specific pipeline configurations
	* @param polygonMode Polygon mode for rasterization
	*/
	void createPipeline(RenderPass& renderPass, vk::Extent2D swapchainExtent, vk::DescriptorSetLayout descriptorSetLayout, PipelineType type, vk::PolygonMode polygonMode);
	
	/**
	* @brief Creates a shader module from the given SPIR-V bytecode
	* 
	* @param inputCode Vector of bytes containing the SPIR-V code for the shader
	* 
	* @return vk::UniqueShaderModule - a unique handle to the created shader module
	*/
	vk::UniqueShaderModule createShaderModule(const std::vector<char>& inputCode);

	Device& m_device;
	vk::UniquePipelineLayout pipelineLayout;
	vk::UniquePipeline graphicsPipeline;
};

/* End of the Pipeline.hpp file */