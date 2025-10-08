#include "ui.hpp"
#include <filesystem>
#include <iostream>
#include "../rendering/Renderer.hpp"

UserInterface::UserInterface(Instance &instance, Device& dev, Swapchain& swapchain, RenderPass& renderPass, Window& window, CommandManager& cmdManager)
	: uiDevice(dev), uiInstance(instance), uiSwapchain(swapchain), uiRenderPass(renderPass), uiWindow(window), uiCmdManager(cmdManager)
{
	scanModels();
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

void UserInterface::beginFrame(std::unique_ptr<Model>& currentModel, Device& device, Renderer& renderer, Transform& transform)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	showStatistics();
	showModelMenu(currentModel, device, renderer, transform);
}

void UserInterface::scanModels()
{
	menuModels.clear();
	std::string path = "../../../assets/";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (entry.is_regular_file())
		{
			std::string filePath = entry.path().string();
			std::string extension = entry.path().extension().string();
			if (extension == ".gltf")
			{
				menuModels.push_back(filePath);
			}
		}
	}
}

void UserInterface::showStatistics()
{
	float fps = ImGui::GetIO().Framerate;
	float delta = ImGui::GetIO().DeltaTime;

	frameTimes.push_back(fps);
	if (frameTimes.size() > 1000)
	{
		frameTimes.pop_front();
	}

	float avgFps = 0.0f;
	float minFps = FLT_MAX;
	float maxFps = 0.0f;
	for (float t : frameTimes)
	{
		avgFps += t;
		if (t < minFps) minFps = t;
		if (t > maxFps) maxFps = t;
	}

	if (!frameTimes.empty())
	{
		avgFps /= frameTimes.size();
	}

	ImGui::SetNextWindowPos(ImVec2(10, 10));
	ImGui::SetNextWindowSize(ImVec2(200, 300));
	ImGui::Begin("Statistics");
	ImGui::Text("FPS: %.1f", fps);
	ImGui::Text("Delta time: %.3f ms", delta * 1000.0f);
	ImGui::Separator();
	ImGui::Text("Average: %.1f");
	ImGui::Text("Min: %.1f", minFps);
	ImGui::Text("Max: %.1f", maxFps);

	if (!frameTimes.empty())
	{
		std::vector<float> frameTimesVec(frameTimes.begin(), frameTimes.end());
		ImGui::PlotLines("Graph", frameTimesVec.data(), (int)frameTimes.size(), 0, nullptr, 0.0f, 200.0f, ImVec2(150, 100));
	}

	ImGui::End();
}

void UserInterface::showModelMenu(std::unique_ptr<Model>& currentModel, Device& device, Renderer& renderer, Transform& transform)  
{  
	ImGui::SetNextWindowPos(ImVec2(10, 350));  
	ImGui::SetNextWindowSize(ImVec2(250, 400));  
	if (ImGui::Begin("Models", nullptr, ImGuiWindowFlags_None))
	{  
		ImGui::Separator();  
		if (ImGui::BeginChild("ModelList", ImVec2(0, 300), true))  
		{  
			for (const auto& modelPath : menuModels)  
			{  
				if (ImGui::Selectable(modelPath.c_str())) {  
					try {  
						currentModel = std::make_unique<Model>(device, modelPath);  
						float setScale = currentModel->getScaleIndex();
						transform.setScale(glm::vec3(setScale, setScale, setScale));
						renderer.setModel(*currentModel);
						std::cout << "Loaded model: " << modelPath << std::endl;  
					}  
					catch (const std::exception& e) {  
						std::cerr << "Failed to load model: " << modelPath << " - " << e.what() << std::endl;  
					}  
				}  
			}  
			ImGui::EndChild();  
		}  
		ImGui::Separator();  
		if (ImGui::Button("Refresh List"))  
		{  
			scanModels();  
		}  
	}  
	ImGui::End();  
}

void UserInterface::handleMouseMove(double x, double y)
{
	ImGui_ImplGlfw_CursorPosCallback(uiWindow.getGLFWWindow(), x, y);
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