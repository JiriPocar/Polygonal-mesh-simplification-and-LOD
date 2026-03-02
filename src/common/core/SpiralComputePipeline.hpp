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

class SpiralComputePipeline {
public:
    SpiralComputePipeline(Device& device, vk::DescriptorSetLayout descSetLayout);

    vk::Pipeline get() const { return pipeline.get(); }
    vk::PipelineLayout getLayout() const { return layout.get(); }

private:
    Device& dev;
    vk::UniquePipelineLayout layout;
    vk::UniquePipeline pipeline;
};