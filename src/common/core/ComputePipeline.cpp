/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file ComputePipeline.cpp
 * @brief Compute pipeline class declaration.
 *
 * This file contains the implementation of the compute pipeline class. Sets up compute pipeline.
 * 
 * Parts of the code may be inspired or adapted from:
 *		- Alexander Overvoorde's "Vulkan Tutorial"
 *			- @url https://vulkan-tutorial.com/
 *			- @url https://github.com/Overv/VulkanTutorial
 *		- Victor Blanco's "Vulkan Guide"
 *			- @url https://vkguide.dev/
 *			- @url https://github.com/vblanco20-1/vulkan-guide
 */

#include "ComputePipeline.hpp"
#include <fstream>

static std::vector<char> readFile(const std::string& filename)
{
    /**
	* Code of this function is directly taken from 'Vulkan tutorial' github repository, file 09_shader_modules.cpp
    * 
    * @author Alexander Overdoore
    * @url https://github.com/Overv/VulkanTutorial/blob/main/code/09_shader_modules.cpp
    * @lines 550-566
    * @licence CC0-1.0
    */
    std::string path = "shaders/" + filename;
    std::ifstream file(path, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

ComputePipeline::ComputePipeline(Device& device, vk::DescriptorSetLayout descSetLayout, uint32_t pushConstantSize)
    : dev(device)
{
    // load compute shader
    auto compCode = readFile("shader.comp.spv");
    vk::ShaderModuleCreateInfo moduleInfo(
        {},
        compCode.size(),
        reinterpret_cast<const uint32_t*>(compCode.data())
    );

	// create shader module
    vk::UniqueShaderModule compModule = dev.operator*().createShaderModuleUnique(moduleInfo);

	// set up compute pipeline
    vk::PipelineShaderStageCreateInfo stageInfo(
        {},
        vk::ShaderStageFlagBits::eCompute,
        *compModule,
        "main"
    );

    vk::PushConstantRange pushRange(
        vk::ShaderStageFlagBits::eCompute,
        0,
        pushConstantSize
    );

	// create pipeline layout with descriptor set layout and push constant range
    vk::PipelineLayoutCreateInfo layoutInfo(
        {},
        1,
        &descSetLayout,
        1,
        &pushRange
    );
    layout = dev.operator*().createPipelineLayoutUnique(layoutInfo);

	// create compute pipeline
    vk::ComputePipelineCreateInfo pipelineInfo(
        {},
        stageInfo,
        *layout
    );
    auto result = dev.operator*().createComputePipelinesUnique(VK_NULL_HANDLE, pipelineInfo);
    pipeline = std::move(result.value[0]);
}

/* End of the ComputePipeline.cpp file */