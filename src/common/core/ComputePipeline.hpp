/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file ComputePipeline.hpp
 * @brief Compute pipeline class declaration.
 *
 * This file contains the implementation of the compute pipeline class. Sets up compute pipeline.
 */

#pragma once
#include <vulkan/vulkan.hpp>
#include "Device.hpp"
#include <vector>

class ComputePipeline {
public:
    /**
	* @brief Constructor for ComputePipeline class. Loads the compute shader, creates the shader module, pipeline object and pipeline layout object.
    * 
	* @param device Reference to the Vulkan device
	* @param descSetLayout Descriptor set layout used by the compute shader
	* @param pushConstantSize Size of the push constants used by the compute shader
    */
    ComputePipeline(Device& device, vk::DescriptorSetLayout descSetLayout, uint32_t pushConstantSize);

    // getters
    vk::Pipeline get() const { return pipeline.get(); }
    vk::PipelineLayout getLayout() const { return layout.get(); }

private:
    Device& dev;
    vk::UniquePipelineLayout layout;
    vk::UniquePipeline pipeline;
};

/* End of the ComputePipeline.hpp file */