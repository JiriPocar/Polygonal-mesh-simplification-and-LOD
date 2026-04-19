/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Pipeline.cpp
 * @brief Pipeline class methods implementation for Vulkan application.
 *
 * This file contains the implementation of the Pipeline class, shader
 * loading and graphics pipeline creation.
 * 
 * Parts of the code may be inspired or adapted from:
 *		- Alexander Overvoorde's "Vulkan Tutorial"
 *			- @url https://vulkan-tutorial.com/
 *			- @url https://github.com/Overv/VulkanTutorial
 *		- Victor Blanco's "Vulkan Guide"
 *			- @url https://vkguide.dev/
 *			- @url https://github.com/vblanco20-1/vulkan-guide
 */

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <filesystem>

#include "Pipeline.hpp"
#include "Device.hpp"
#include "../rendering/RenderPass.hpp"

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

Pipeline::Pipeline(Device& device,
	RenderPass& renderPass,
	vk::Extent2D swapchainExtent,
	vk::DescriptorSetLayout descriptorSetLayout,
	PipelineType type,
	vk::PolygonMode polygonMode)
	: pipelineDevice(device)
{
	createPipeline(renderPass, swapchainExtent, descriptorSetLayout, type, polygonMode);
}

vk::UniqueShaderModule Pipeline::createShaderModule(const std::vector<char>& inputCode)
{
	vk::ShaderModuleCreateInfo createInfo(
		{},
		inputCode.size(),
		reinterpret_cast<const uint32_t*>(inputCode.data())
	);

	return pipelineDevice.operator*().createShaderModuleUnique(createInfo);
}

void Pipeline::createPipeline(RenderPass& renderPass, vk::Extent2D swapchainExtent, vk::DescriptorSetLayout descriptorSetLayout, PipelineType type, vk::PolygonMode polygonMode)
{
	// load shaders
	auto vert = readFile("shader.vert.spv");
	auto frag = readFile("shader.frag.spv");
	auto vertShaderModule = createShaderModule(vert);
	auto fragShaderModule = createShaderModule(frag);

	// vertex shader
	vk::PipelineShaderStageCreateInfo vertShaderStageInfo(
		{},
		vk::ShaderStageFlagBits::eVertex,
		*vertShaderModule,
		"main"
	);
	vk::PipelineShaderStageCreateInfo fragShaderStageInfo(
		{},
		vk::ShaderStageFlagBits::eFragment,
		*fragShaderModule,
		"main"
	);
	vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
		{},						// flags
		1,						// set layouts
		&descriptorSetLayout,	// descriptor sets
		0,
		nullptr					// no push constants
	);
	pipelineLayout = pipelineDevice.operator*().createPipelineLayoutUnique(pipelineLayoutInfo);

	// vertex input
	std::vector<vk::VertexInputBindingDescription> bindings;
	std::vector<vk::VertexInputAttributeDescription> attributes;

	bindings.push_back(Vertex::getBindingDesc());
	auto vertAttrs = Vertex::getAttributeDesc();
	attributes.insert(attributes.end(), vertAttrs.begin(), vertAttrs.end());

	if (type == PipelineType::Spiral)
	{
		vk::VertexInputBindingDescription instanceBindingDescription(
			1,
			16, // stride = sizeof(SpiralInstaceData) = 16 bytes
			vk::VertexInputRate::eInstance
		);
		bindings.push_back(instanceBindingDescription);

		std::array<vk::VertexInputAttributeDescription, 2> instanceAttributes{};

		// location 3, instance position
		instanceAttributes[0].binding = 1;
		instanceAttributes[0].location = 3;
		instanceAttributes[0].format = vk::Format::eR32G32B32Sfloat;
		instanceAttributes[0].offset = 0; // start at beginning of instance data

		// location 4, instance model type
		instanceAttributes[1].binding = 1;
		instanceAttributes[1].location = 4;
		instanceAttributes[1].format = vk::Format::eR32Uint;
		instanceAttributes[1].offset = 12; // offset of sizeof(vec3) in instance data

		attributes.push_back(instanceAttributes[0]);
		attributes.push_back(instanceAttributes[1]);
	}

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
		{},
		static_cast<uint32_t>(bindings.size()),
		bindings.data(),
		static_cast<uint32_t>(attributes.size()),
		attributes.data()
	);

	// input assembly
	vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
		{},										// flags
		vk::PrimitiveTopology::eTriangleList,	// draw triangles
		VK_FALSE								// no restart
	);

	// viewport and scissors
	vk::Viewport viewport(
		0.0f, 0.0f,	// x y
		static_cast<float>(swapchainExtent.width),
		static_cast<float>(swapchainExtent.height),
		0.0f, 1.0f  // min max depth
	);

	vk::Rect2D scissor(
		{ 0, 0 }, // offset
		swapchainExtent
	);

	vk::PipelineViewportStateCreateInfo viewportState(
		{},				// flags
		1, &viewport,	// viewport count and data
		1, &scissor		// scissor count and data
	);

	// rasterizer
	vk::PipelineRasterizationStateCreateInfo rasterizer(
		{},							// flags
		VK_FALSE,					// depth clamp
		VK_FALSE,					// rasterizer discard
		polygonMode,				// polygon mode
		vk::CullModeFlagBits::eFront,
		vk::FrontFace::eClockwise,
		VK_FALSE,					// depth bias
		0.0f, 0.0f, 0.0f,			// depth bias params
		1.0f						// line width
	);

	// multisampling
	vk::PipelineMultisampleStateCreateInfo multisampling(
		{},									// flags
		vk::SampleCountFlagBits::e1,		// no multisampling
		VK_FALSE,							// no sample shading
		1.0f,								// min sample shading
		nullptr,							// no sample mask
		VK_FALSE,							// no alpha to coverage
		VK_FALSE							// no alpha to one
	);

	// color blending
	vk::PipelineColorBlendAttachmentState colorBlendingAttach(
		VK_FALSE,
		vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
		vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
		vk::ColorComponentFlagBits::eR |
		vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB |
		vk::ColorComponentFlagBits::eA
	);

	vk::PipelineColorBlendStateCreateInfo colorBlending(
		{},					 // flags
		VK_FALSE,			 // no logic op
		vk::LogicOp::eCopy,  // logic op
		1,					 // attachment count
		&colorBlendingAttach // attachment states
	);

	vk::PipelineDepthStencilStateCreateInfo depthStencil(
		{},
		VK_TRUE,				// depthTestEnable
		VK_TRUE,				// depthWriteEnable
		vk::CompareOp::eLess,	// depthCompareOp
		VK_FALSE,				// depthBoundsTestEnable
		VK_FALSE,				// stencilTestEnable
		{},
		{},
		0.0f,
		1.0f
	);

	// dynamic state for viewport and scissor
	std::vector<vk::DynamicState> dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	vk::PipelineDynamicStateCreateInfo dynamicStateInfo(
		{},
		static_cast<uint32_t>(dynamicStates.size()),
		dynamicStates.data()
	);

	vk::PipelineDynamicStateCreateInfo* pDynamicState = nullptr;
	if (type == PipelineType::Simplificator)
	{
		pDynamicState = &dynamicStateInfo;
	}

	// create the graphics pipeline
	vk::GraphicsPipelineCreateInfo pipelineInfo(
		{},						// flags
		2, shaderStages,		// shader stages
		&vertexInputInfo,		// vertex input
		&inputAssembly,			// input assembly
		nullptr,				// tessellation
		&viewportState,			// viewport state
		&rasterizer,			// rasterizer
		&multisampling,			// multisampling
		&depthStencil,			// depth stencil
		&colorBlending,			// color blending
		pDynamicState,			// dynamic state
		*pipelineLayout,		// pipeline layout
		renderPass.get(),		// render pass
		0,						// subpass
		VK_NULL_HANDLE,			// base pipeline handle
		-1						// base pipeline index
	);

	auto result = pipelineDevice.operator*().createGraphicsPipelineUnique(VK_NULL_HANDLE, pipelineInfo);
	if (result.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to create graphics pipeline");
	}

	graphicsPipeline = std::move(result.value);
}

/* End of the Pipeline.cpp file */