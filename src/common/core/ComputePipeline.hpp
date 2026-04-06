/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file ComputePipeline.hpp
 * @brief Compute pipeline class declaration for Spiral application.
 *
 * This file contains the declaration of the Pipeline class.
 */

#pragma once
#include <vulkan/vulkan.hpp>
#include "Device.hpp"
#include <vector>

struct ComputePushConstants {
    uint32_t totalInstances;
    float lodDist0;
    float lodDist1;
    float lodDist2;
    float lodDist3;
    uint32_t computeSpiral;
    float spacing;
    int numArms;
    float minRadius;
    float coneFactor;
    float twistSpeed;
    float animationTime;
    uint32_t enableLOD;
};

class ComputePipeline {
public:
    ComputePipeline(Device& device, vk::DescriptorSetLayout descSetLayout, uint32_t pushConstantSize);

    vk::Pipeline get() const { return pipeline.get(); }
    vk::PipelineLayout getLayout() const { return layout.get(); }

private:
    Device& dev;
    vk::UniquePipelineLayout layout;
    vk::UniquePipeline pipeline;
};

/* End of the ComputePipeline.hpp file */