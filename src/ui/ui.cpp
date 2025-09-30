#include "ui.hpp"

UserInterface::UserInterface(Instance &instance, Device& dev, Swapchain& swapchain, RenderPass& renderPass, Window& window, CommandManager& cmdManager)
	: uiDevice(dev), uiInstance(instance), uiSwapchain(swapchain), uiRenderPass(renderPass), uiWindow(window), uiCmdManager(cmdManager)
{
	init();
}

UserInterface::~UserInterface()
{
	cleanUp();
}

void UserInterface::init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsClassic();
	ImGui_ImplGlfw_InitForVulkan(uiWindow.getGLFWWindow(), true);

	createDescriptorPool();

	ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = uiInstance.get();
		initInfo.PhysicalDevice = uiDevice.getPhysicalDevice();
		initInfo.Device = uiDevice.operator*();
		initInfo.QueueFamily = uiDevice.getGraphicsQueueFamily();
		initInfo.Queue = uiDevice.getGraphicsQueue();
		initInfo.PipelineCache = nullptr;
		initInfo.DescriptorPool = *descriptorPool;
		initInfo.Subpass = 0;
		initInfo.MinImageCount = 2;
		initInfo.ImageCount = static_cast<uint32_t>(uiSwapchain.getImages().size());
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.RenderPass = uiRenderPass.get();

	ImGui_ImplVulkan_Init(&initInfo);

}

void UserInterface::createDescriptorPool()
{
	std::vector<vk::DescriptorPoolSize> poolSizes = {
		{ vk::DescriptorType::eSampler, 1000 },
		{ vk::DescriptorType::eCombinedImageSampler, 1000 },
		{ vk::DescriptorType::eSampledImage, 1000 },
		{ vk::DescriptorType::eStorageImage, 1000 },
		{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
		{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
		{ vk::DescriptorType::eUniformBuffer, 1000 },
		{ vk::DescriptorType::eStorageBuffer, 1000 },
		{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
		{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
		{ vk::DescriptorType::eInputAttachment, 1000 }
	};

	vk::DescriptorPoolCreateInfo poolInfo(
		vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		2000,
		static_cast<uint32_t>(poolSizes.size()),
		poolSizes.data()
	);

	descriptorPool = uiDevice.operator*().createDescriptorPoolUnique(poolInfo);
}

void UserInterface::beginFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Example window
	ImGui::Begin("Example Window");
	ImGui::Text("Hello, Vulkan ImGui!");
	ImGui::End();
}

void UserInterface::render(vk::CommandBuffer cmdBuffer)
{
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmdBuffer);
}

void UserInterface::cleanUp()
{
	uiDevice.operator*().waitIdle();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}