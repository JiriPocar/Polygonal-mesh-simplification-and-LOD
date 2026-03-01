#include "SpiralComputePipeline.hpp"
#include <fstream>

static std::vector<char> readFile(const std::string& filename)
{
    std::string path = "shaders/" + filename;
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file) {
        throw std::runtime_error("Failed to open shader file: " + path);
    }

    std::streamsize size = file.tellg();
    if (size <= 0) {
        throw std::runtime_error("Shader file is empty or unreadable: " + path);
    }

    std::vector<char> buffer(static_cast<size_t>(size));
    file.seekg(0);
    file.read(buffer.data(), size);

    return buffer;
}

SpiralComputePipeline::SpiralComputePipeline(Device& device, vk::DescriptorSetLayout descSetLayout)
    : dev(device)
{
    auto compCode = readFile("shader.comp.spv");

    vk::ShaderModuleCreateInfo moduleInfo({}, compCode.size(), reinterpret_cast<const uint32_t*>(compCode.data()));
    vk::UniqueShaderModule compModule = dev.operator*().createShaderModuleUnique(moduleInfo);
    vk::PipelineShaderStageCreateInfo stageInfo({}, vk::ShaderStageFlagBits::eCompute, *compModule, "main");
    vk::PushConstantRange pushRange(vk::ShaderStageFlagBits::eCompute, 0, sizeof(ComputePushConstants));

    vk::PipelineLayoutCreateInfo layoutInfo({}, 1, &descSetLayout, 1, &pushRange);
    layout = dev.operator*().createPipelineLayoutUnique(layoutInfo);

    vk::ComputePipelineCreateInfo pipelineInfo({}, stageInfo, *layout);
    auto result = dev.operator*().createComputePipelinesUnique(VK_NULL_HANDLE, pipelineInfo);
    pipeline = std::move(result.value[0]);
}