#include "ui.hpp"
#include <filesystem>
#include <iostream>
#include <chrono>
#include "../rendering/Renderer.hpp"
#include "../rendering/SpiralRenderer.hpp"
#include "../scene/SpiralScene.hpp"

UserInterface::UserInterface(Instance &instance, Device& dev, Swapchain& swapchain, RenderPass& renderPass, Window& window, CommandManager& cmdManager)
	: uiDevice(dev), uiInstance(instance), uiSwapchain(swapchain), uiRenderPass(renderPass), uiWindow(window), uiCmdManager(cmdManager), simplificator()
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

void UserInterface::beginFrame(std::unique_ptr<DualModel>& currentDualModel, Device& device, Renderer& renderer, Transform& transform)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::StyleColorsDark();

	showStatistics();
	showModelMenu(currentDualModel, device, renderer, transform);
	showSimplificationControls(currentDualModel, device);
	showModelPerspectiveControls(transform);
	showWireframeControls(renderer);
	showSmoothingControls();
}

void UserInterface::beginFrame2(SpiralScene& spiral, SpiralRenderer& renderer)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::StyleColorsDark();

	showStatistics();
	showSpiralControls(spiral);
	showGeneralControls(spiral);
	showWireframeControls2(renderer);
}

void UserInterface::scanModels()
{
	menuModels.clear();
	std::string path = "assets/";
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (entry.is_regular_file())
		{
			std::string filePath = entry.path().string();
			std::string extension = entry.path().extension().string();
			if (extension == ".gltf")
			{
				menuModels.push_back(filePath);
				std::cout << "Found model: " << filePath << std::endl;
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

void UserInterface::showModelMenu(std::unique_ptr<DualModel>& currentDualModel, Device& device, Renderer& renderer, Transform& transform)  
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
						device.operator*().waitIdle(); // wait for device to be idle before loading new model
						currentDualModel = std::make_unique<DualModel>(device, uiCmdManager, modelPath);  
						float setScale = currentDualModel->getOriginalModel().getScaleIndex();
						transform.setScale(glm::vec3(setScale, setScale, setScale));
						renderer.setDualModel(*currentDualModel);
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

void UserInterface::showSimplificationControls(std::unique_ptr<DualModel>& currentDualModel, Device& device)
{
	if (!currentDualModel)
	{
		ImGui::Begin("Simplification");
		ImGui::Text("No model loaded.");
		ImGui::End();
		return;
	}

	ImGui::SetNextWindowPos(ImVec2(10, 770));
	ImGui::SetNextWindowSize(ImVec2(300, 120));

	ImGui::Begin("Simplification");

	// static here, so the value persists between frames
	static float ratio = 0.5f;
	static int cellsPerAxis = 10;
	Algorithm currentAlgorithm = simplificator.getCurrentAlgorithm();

	// provisional solution, TODO: make this quite more elegant
	const char* algorithms[] = { "QEM", "Vertex Clustering", "Floating-cell clustering", "Vertex Decimation", "Naive"};
	if (ImGui::BeginCombo("Algorithm", algorithms[static_cast<int>(currentAlgorithm)]))
	{
		for (int i = 0; i < 4; i++)
		{
			if (ImGui::Selectable(algorithms[i], currentAlgorithm == static_cast<Algorithm>(i)))
			{
				currentAlgorithm = static_cast<Algorithm>(i);
				simplificator.setCurrentAlgorithm(currentAlgorithm);
			}
		}

		ImGui::EndCombo();
	}

	currentAlgorithm = simplificator.getCurrentAlgorithm();
	float parameterValue = 0.0f;

	if (currentAlgorithm != Algorithm::VertexClustering)
	{
		ImGui::SliderFloat("Reduction Ratio", &ratio, 0.01f, 0.9f, "%.2f");
		parameterValue = ratio;
	}
	else
	{
		// slider for number of cells per axis
		ImGui::SliderInt("Cells Per Axis", &cellsPerAxis, 2, 100);
		parameterValue = static_cast<float>(cellsPerAxis);

		// choose clustering method
		ClusteringMethod currentMethod = simplificator.getClusteringMethod();
		const char* strategies[] = { "Cell Center", "Quadric Error Metric", "Highest Weight", "Mean Weight"};

		if (ImGui::BeginCombo("Method", strategies[static_cast<int>(currentMethod)]))
		{
			for (int i = 0; i < 4; i++) {
				if (ImGui::Selectable(strategies[i], currentMethod == static_cast<ClusteringMethod>(i)))
				{
					currentMethod = static_cast<ClusteringMethod>(i);
					simplificator.setClusteringMethod(currentMethod);
				}
			}

			ImGui::EndCombo();
		}
	}
	

	auto& originalModel = currentDualModel->getOriginalModel();
	
	if (ImGui::Button("Apply"))
	{
		try {
			auto result = simplificator.simplify(originalModel, parameterValue);

			device.operator*().waitIdle(); // wait for device to be idle before applying simplification
			currentDualModel->simplifyModel(result.vertices, result.indices);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Simplification failed: " << e.what() << std::endl;
		}
	}

	if (currentDualModel->wasModelSimplified())
	{
		if (ImGui::Button("Revert"))
		{
			device.operator*().waitIdle(); // wait for device to be idle before reverting
			currentDualModel->revertSimplification();
		}
	}

	ImGui::End();
}

void UserInterface::showModelPerspectiveControls(Transform& transform)
{
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(420, 10));
	ImGui::SetNextWindowSize(ImVec2(180, 180));

	ImGui::Begin("Model Transform");
	glm::vec3 rotation = transform.getRot();
	if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f))
	{
		uiTransform.setRot(rotation);
	}

	// checkboxes for rotation by axis
	ImGui::Checkbox("X", &rotateByX);
	ImGui::Checkbox("Y", &rotateByY);
	ImGui::Checkbox("Z", &rotateByZ);

	// reset transform
	if (ImGui::Button("Reset Transform"))
	{
		uiTransform = Transform();
	}

	ImGui::End();
}

Transform UserInterface::fetchTransform()
{
	return uiTransform;
}

void UserInterface::setTransform(const Transform& transform)
{
	uiTransform = transform;
}

void UserInterface::showSmoothingControls()
{
	// enable flat shading
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(620, 10));
	ImGui::SetNextWindowSize(ImVec2(180, 60));
	ImGui::Begin("Shading");

	static bool flatShading = true;
	if (ImGui::Checkbox("Flat Shading", &flatShading))
	{
		simplificator.enableFlatShading(flatShading);
	}
	
	ImGui::End();
}

void UserInterface::showWireframeControls(Renderer& renderer)
{
	static bool wireframe = false;
	
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(220, 10));
	ImGui::SetNextWindowSize(ImVec2(180, 60));

	ImGui::Begin("Wireframe");
	if (ImGui::Checkbox("Show wireframe", &wireframe))
	{
		renderer.setShowWireframe(wireframe);
	}
	ImGui::End();
}

void UserInterface::showWireframeControls2(SpiralRenderer& renderer)
{
	static bool wireframe = false;

	ImGui::SetNextWindowCollapsed(true, ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(220, 10));
	ImGui::SetNextWindowSize(ImVec2(180, 60));

	ImGui::Begin("Wireframe");
	if (ImGui::Checkbox("Show wireframe", &wireframe))
	{
		renderer.setShowWireframe(wireframe);
	}
	ImGui::End();
}

void UserInterface::showGeneralControls(SpiralScene& scene)
{
	ImGui::SetNextWindowPos(ImVec2(10, 300));
	ImGui::SetNextWindowSize(ImVec2(200, 250));

	ImGui::Begin("General Settings");
	static int selectedInstanceCount = scene.config.instanceCount;
	ImGui::InputInt("Count", &selectedInstanceCount, 100, 1000);
	if (ImGui::Button("Apply"))
	{
		if (selectedInstanceCount < 0) selectedInstanceCount = 0;

		if (selectedInstanceCount > static_cast<int>(scene.getMaxInstanceCount()))
			selectedInstanceCount = scene.getMaxInstanceCount();

		scene.config.instanceCount = static_cast<uint32_t>(selectedInstanceCount);
		scene.updateSpiralPositions(0.0f); // reset positions
	}

	ImGui::End();
}

void UserInterface::showSpiralControls(SpiralScene& scene)
{
	ImGui::SetNextWindowPos(ImVec2(10, 660));
	ImGui::SetNextWindowSize(ImVec2(400, 300));

	ImGui::Begin("Spiral settings");

	// shape
	ImGui::Text("Shape Parameters");
	ImGui::SliderFloat("Spacing", &scene.config.spacing, 0.5f, 20.0f);
	ImGui::SliderInt("Arms (Num)", &scene.config.numArms, 1, 12);
	ImGui::SliderFloat("Min Radius", &scene.config.minRadius, 0.0f, 100.0f);
	ImGui::SliderFloat("Cone Factor", &scene.config.coneFactor, 0.0f, 2.0f);
	ImGui::Separator();

	// animation
	ImGui::Text("Animation Parameters");
	ImGui::SliderFloat("Speed", &scene.config.speed, 0.0f, 500.0f);
	ImGui::SliderFloat("Twist Speed", &scene.config.twistSpeed, 0.0f, 0.2f);
	ImGui::SliderFloat("Arm Rotation", &scene.config.armSpread, 0.0f, 6.28f);
	ImGui::Separator();

	// reset
	if (ImGui::Button("Reset Animation Time")) {
		scene.resetAnimation();
	}

	// debug
	ImGui::Text("Instance Count: %d", scene.config.instanceCount);
	ImGui::Text("Total Length: %.2f", scene.config.instanceCount * scene.config.spacing);

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