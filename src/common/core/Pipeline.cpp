/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file Pipeline.cpp
 * @brief Pipeline class methods implementation for Vulkan application.
 *
 * This file contains the implementation of the Pipeline class, shader
 * loading and graphics pipeline creation.
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

Pipeline::Pipeline(Device& device, RenderPass& renderPass, vk::Extent2D swapchainExtent, vk::DescriptorSetLayout descriptorSetLayout, vk::PolygonMode polygonMode)
	: pipelineDevice(device)
{
	createPipeline(renderPass, swapchainExtent, descriptorSetLayout, polygonMode);
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

void Pipeline::createPipeline(RenderPass& renderPass, vk::Extent2D swapchainExtent, vk::DescriptorSetLayout descriptorSetLayout, vk::PolygonMode polygonMode)
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

	// vertex input
	auto bindingDescription = Vertex::getBindingDesc();
	auto attributeDescriptions = Vertex::getAttributeDesc();

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
		{},
		1, &bindingDescription,
		static_cast<uint32_t>(attributeDescriptions.size()), attributeDescriptions.data()
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

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
		{},						// flags
		1,						// set layouts
		&descriptorSetLayout,	// descriptor sets
		0,
		nullptr					// no push constants
	);

	pipelineLayout = pipelineDevice.operator*().createPipelineLayoutUnique(pipelineLayoutInfo);

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
		nullptr,				// dynamic state
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