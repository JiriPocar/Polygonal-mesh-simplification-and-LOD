#include "SpiralPipeline.hpp"
#include <fstream>
#include <stdexcept>
#include <iostream>

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

SpiralPipeline::SpiralPipeline(Device& device,
    RenderPass& renderPass,
    vk::Extent2D swapchainExtent,
    vk::DescriptorSetLayout descriptorSetLayout)
    : pipelineDevice(device)
{
    createPipeline(renderPass, swapchainExtent, descriptorSetLayout);
}

vk::UniqueShaderModule SpiralPipeline::createShaderModule(const std::vector<char>& inputCode)
{
    vk::ShaderModuleCreateInfo createInfo(
        {},
        inputCode.size(),
        reinterpret_cast<const uint32_t*>(inputCode.data())
    );

    return pipelineDevice.operator*().createShaderModuleUnique(createInfo);
}

void SpiralPipeline::createPipeline(const RenderPass& renderPass,
    vk::Extent2D swapchainExtent,
    vk::DescriptorSetLayout descSetLayout)
{
    // load shaders
    auto vert = readFile("shader.vert.spv");
    auto frag = readFile("shader.frag.spv");
    auto vertShaderModule = createShaderModule(vert);
    auto fragShaderModule = createShaderModule(frag);

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
	vk::VertexInputBindingDescription vertexBindingDescription(
		0,
		sizeof(Vertex),
		vk::VertexInputRate::eVertex
	);

	vk::VertexInputBindingDescription instanceBindingDescription(
		1,
		32, // stride = sizeof(SpiralInstaceData) = 32 bytes
		vk::VertexInputRate::eInstance
	);

    std::array<vk::VertexInputBindingDescription, 2> bindingDescription = {
        vertexBindingDescription,
		instanceBindingDescription
    };

	auto vertexAttributes = Vertex::getAttributeDesc();
    std::array<vk::VertexInputAttributeDescription, 3> instanceAttributes{};

	// location 3, instance position
    instanceAttributes[0].binding = 1;
	instanceAttributes[0].location = 3;
	instanceAttributes[0].format = vk::Format::eR32G32B32Sfloat;
	instanceAttributes[0].offset = 0;

	// location 4, instance model type
	instanceAttributes[1].binding = 1;
	instanceAttributes[1].location = 4;
	instanceAttributes[1].format = vk::Format::eR32Uint;
	instanceAttributes[1].offset = 16;

	// location 5, instance LOD level
	instanceAttributes[2].binding = 1;
	instanceAttributes[2].location = 5;
	instanceAttributes[2].format = vk::Format::eR32Uint;
	instanceAttributes[2].offset = 20;

	std::vector<vk::VertexInputAttributeDescription> allAttributes;
	allAttributes.insert(allAttributes.end(), vertexAttributes.begin(), vertexAttributes.end());
	allAttributes.insert(allAttributes.end(), instanceAttributes.begin(), instanceAttributes.end());

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
		{},
		static_cast<uint32_t>(bindingDescription.size()),
		bindingDescription.data(),
		static_cast<uint32_t>(allAttributes.size()),
		allAttributes.data()
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
		{},								// flags
		VK_FALSE,						// depth clamp
		VK_FALSE,						// rasterizer discard
		vk::PolygonMode::eLine,			// polygon mode
		vk::CullModeFlagBits::eFront,	// cull front faces
		vk::FrontFace::eClockwise,		// front faces clockwise
		VK_FALSE,						// depth bias
		0.0f, 0.0f, 0.0f,				// depth bias params
		1.0f							// line width
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

	vk::PushConstantRange pushConstantsRange(
		vk::ShaderStageFlagBits::eVertex,
		0,
		sizeof(uint32_t)
	);

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo(
		{},						// flags
		1,						// set layouts
		&descSetLayout,			// descriptor sets
		1,
		&pushConstantsRange		// push constants
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