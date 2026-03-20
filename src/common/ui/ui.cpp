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

void UserInterface::beginFrame(std::unique_ptr<DualModel>& currentDualModel, Device& device, Renderer& renderer, Transform& transform, bool show)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::StyleColorsDark();

	if (show)
	{
		showModelMenu(currentDualModel, device, renderer, transform);
		showSimplificationControls(currentDualModel, device);
		showSimplificationResults();
		showModelPerspectiveControls(transform);
		showWireframeControls(renderer);
		showSmoothingControls();
	}
	
}

void UserInterface::beginFrame2(SpiralScene& spiral, SpiralRenderer& renderer, bool show)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::StyleColorsDark();

	if (show)
	{
		showStatistics();
		showSpiralControls(spiral);
		showGeneralControls(spiral, renderer);
		showWireframeControls2(renderer);
		showUseGPUCPUControls(renderer);
		showSceneInfo(spiral);
	}
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
	ImGui::SetNextWindowSize(ImVec2(200, 235));
	ImGui::Begin("Statistics");
	ImGui::Text("FPS: %.1f", fps);
	ImGui::Text("Delta time: %.3f ms", delta * 1000.0f);
	ImGui::Separator();
	ImGui::Text("Average: %.1f", avgFps);
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
	ImGui::SetNextWindowPos(ImVec2(10, 490));
	ImGui::SetNextWindowSize(ImVec2(250, 400));
	ImGui::Begin("Models");

	ImGui::Separator();

	if (ImGui::BeginChild("ModelList", ImVec2(0, 300), true))
	{
		for (const auto& modelPath : menuModels)
		{
			if (ImGui::Selectable(modelPath.c_str()))
			{
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

	ImGui::SetNextWindowPos(ImVec2(1490, 10));
	ImGui::SetNextWindowSize(ImVec2(300, 355));

	ImGui::Begin("Simplification");

	// static here, so the value persists between frames
	static float ratio = 0.5f;
	static int cellsPerAxis = 10;
	Algorithm currentAlgorithm = simplificator.getCurrentAlgorithm();

	// provisional solution, TODO: make this quite more elegant
	const char* algorithms[] = { "QEM", "Vertex Clustering", "Floating-cell clustering", "Vertex Decimation", "Naive"};
	if (ImGui::BeginCombo("Algorithm", algorithms[static_cast<int>(currentAlgorithm)]))
	{
		for (int i = 0; i < 5; i++)
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

	if (currentAlgorithm != Algorithm::VertexClustering && currentAlgorithm != Algorithm::FloatingCellClustering)
	{
		ImGui::SliderFloat("Reduction (%)", &ratio, 0.01f, 0.99f, "%.2f");
		parameterValue = ratio;
	}
	else
	{
		// slider for number of cells per axis
		ImGui::SliderInt("Cells Per Axis", &cellsPerAxis, 2, 100);
		parameterValue = static_cast<float>(cellsPerAxis);

		if (currentAlgorithm == Algorithm::VertexClustering)
		{
			// choose clustering method
			ClusteringMethod currentMethod = simplificator.getClusteringMethod();
			const char* method[] = { "Cell Center", "Quadric Error Metric", "Highest Weight", "Mean Weight"};

			if (ImGui::BeginCombo("Method", method[static_cast<int>(currentMethod)]))
			{
				for (int i = 0; i < 4; i++) {
					if (ImGui::Selectable(method[i], currentMethod == static_cast<ClusteringMethod>(i)))
					{
						currentMethod = static_cast<ClusteringMethod>(i);
						simplificator.setClusteringMethod(currentMethod);
					}
				}

				ImGui::EndCombo();
			}
		}
		
	}
	
	ImGui::Separator();

	if (currentAlgorithm != Algorithm::VertexClustering && currentAlgorithm != Algorithm::FloatingCellClustering)
	{
		ImGui::Text("Edge collapse constraints");
		ImGui::Checkbox("Check face flipping", &simplificator.options.checkFaceFlipping);
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Prevents collapses that would cause faces to flip their normal direction, which can lead to visual artifacts.");
		ImGui::Checkbox("Check connectivity", &simplificator.options.checkConnectivity);
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Ensures that collapses do not disconnect the mesh, which can create holes or separate parts of the model.");
		ImGui::Checkbox("Preserve borders", &simplificator.options.preserveBorders);
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Prevents collapsing edges that are on the border of the mesh, maintaining the overall shape.");
		ImGui::Separator();

		ImGui::Text("Holes and tearing prevention");
		ImGui::Checkbox("Simplify with UV seams", &simplificator.options.resolveUVSeams);
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("The simplification process will attempt to preserve UV seams. Use for models with textures.");
		if (ImGui::Checkbox("Enable merging vertices", &simplificator.options.enableMerging))
		{
			if (simplificator.options.enableMerging)
			{
				simplificator.options.mergeCloseVertivesPos = true;
			}
			else
			{
				simplificator.options.mergeCloseVertivesPos = false;
				simplificator.options.mergeCloseVerticesUV = false;
				simplificator.options.mergeCloseVerticesNormal = false;
			}
		}
		if (ImGui::IsItemHovered()) ImGui::SetTooltip("Vertices with the selected criteria will be merged. Use when textures dont matter.");

		ImGui::Indent();

		if (ImGui::Checkbox("Merge by position", &simplificator.options.mergeCloseVertivesPos))
		{
			if (simplificator.options.mergeCloseVertivesPos) simplificator.options.enableMerging = true;
		}

		if (ImGui::Checkbox("Merge by UV", &simplificator.options.mergeCloseVerticesUV))
		{
			if (simplificator.options.mergeCloseVerticesUV)
			{
				simplificator.options.enableMerging = true;
				simplificator.options.mergeCloseVertivesPos = true;
			}
		}

		if (ImGui::Checkbox("Merge by normal", &simplificator.options.mergeCloseVerticesNormal))
		{
			if (simplificator.options.mergeCloseVerticesNormal)
			{
				simplificator.options.enableMerging = true;
				simplificator.options.mergeCloseVertivesPos = true;
			}
		}
		ImGui::Unindent();
	}
	else
	{
		ImGui::Text("By definition clustering algorithms do not consider topology or face flipping");
	}
	

	ImGui::Separator();

	auto& originalModel = currentDualModel->getOriginalModel();
	if (ImGui::Button("Apply"))
	{
		try {
			lastResult = simplificator.simplify(originalModel, parameterValue);
			hasSimplificationResult = true;

			device.operator*().waitIdle(); // wait for device to be idle before applying simplification
			currentDualModel->simplifyModel(lastResult.meshesData);
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

void UserInterface::showSimplificationResults()
{
	ImGui::SetNextWindowPos(ImVec2(1490, 670));
	ImGui::SetNextWindowSize(ImVec2(300, 220));

	ImGui::Begin("Simplification Results");

	if (hasSimplificationResult)
	{
		ImGui::Text("Algorithm:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", getAlgName(lastResult.algorithmUsed));

		ImGui::Separator();

		ImGui::Text("Time taken:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "%.2f ms", lastResult.timeTaken);

		ImGui::Separator();

		ImGui::Text("Original faces:");
		ImGui::SameLine();
		ImGui::Text("%zu", lastResult.originalFaceCount);

		ImGui::Text("Final faces:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "%zu", lastResult.simplifiedFaceCount);

		if (lastResult.originalFaceCount > 0)
		{
			float reduction = 100.0f * (1.0f - (float)lastResult.simplifiedFaceCount / lastResult.originalFaceCount);
			ImGui::Text("Reduction:");
			ImGui::SameLine();
			ImGui::Text("%.2f %%", reduction);
		}

		ImGui::Separator();

		ImGui::Text("Original Vertices:");
		ImGui::SameLine();
		ImGui::Text("%zu", lastResult.originalVertexCount);

		ImGui::Text("Final Vertices:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "%zu", lastResult.simplifiedVertexCount);

		ImGui::Separator();

		float origMB = (float)lastResult.originalMemoryBytes / (1024.0f * 1024.0f);
		float simpMB = (float)lastResult.simplifiedMemoryBytes / (1024.0f * 1024.0f);

		ImGui::Text("Original Memory:");
		ImGui::SameLine();
		ImGui::Text("%.2f MB", origMB);

		ImGui::Text("Final Memory:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "%.2f MB", simpMB);
	}
	else
	{
		ImGui::TextDisabled("No simplification performed yet.");
	}

	ImGui::End();
}

void UserInterface::showModelPerspectiveControls(Transform& transform)
{
	ImGui::SetNextWindowPos(ImVec2(10, 10));
	ImGui::SetNextWindowSize(ImVec2(180, 155));

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

	ImGui::Separator();

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
	ImGui::SetNextWindowPos(ImVec2(10, 175));
	ImGui::SetNextWindowSize(ImVec2(180, 60));
	ImGui::Begin("Shading");

	static bool flatShading = false;
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

void UserInterface::showUseGPUCPUControls(SpiralRenderer& renderer)
{
	ImGui::SetNextWindowPos(ImVec2(1550, 10), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(260, 80), ImGuiCond_Once);

	ImGui::Begin("Compute controls");

	bool useGPULOD = renderer.getUseGPULODCompute();
	if (ImGui::Checkbox("GPU LOD selection compute", &useGPULOD))
	{
		renderer.setUseGPULODCompute(useGPULOD);
	}

	bool useGPUSpiral = renderer.getUseGPUSpiralCompute();
	if (ImGui::Checkbox("GPU Spiral positions compute", &useGPUSpiral))
	{
		renderer.setUseGPUSpiralCompute(useGPUSpiral);
	}

	ImGui::End();
}

void UserInterface::showSceneInfo(SpiralScene& scene)
{
	ImGui::SetNextWindowPos(ImVec2(1505, 670));
	ImGui::SetNextWindowSize(ImVec2(300, 300));

	ImGui::Begin("Scene Info");

	ImGui::Text("Instance Count:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%d", scene.config.instanceCount);

	ImGui::Text("Total Length:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%.2f units", scene.config.instanceCount * scene.config.spacing);

	ImGui::Separator();

	try
	{
		auto& lodSet = scene.getModelLODSet(0);

		ImGui::Text("LOD Mesh Statistics:");

		if (ImGui::BeginTable("LODStats", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
		{
			ImGui::TableSetupColumn("LOD Level");
			ImGui::TableSetupColumn("Vertices");
			ImGui::TableSetupColumn("Faces");
			ImGui::TableHeadersRow();

			for (int i = 0; i < 4; ++i)
			{
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("LOD %d", i);

				uint32_t vCount = lodSet.getLOD(i).getVertexCount();
				uint32_t iCount = lodSet.getLOD(i).getIndexCount();
				uint32_t fCount = iCount / 3;

				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%u", vCount);

				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%u", fCount);
			}
			ImGui::EndTable();
		}
	}
	catch (...)
	{
		ImGui::TextDisabled("Model data not available yet.");
	}

	ImGui::End();
}

void UserInterface::showGeneralControls(SpiralScene& scene, SpiralRenderer& renderer)
{
	ImGui::SetNextWindowPos(ImVec2(10, 250));
	ImGui::SetNextWindowSize(ImVec2(330, 400));

	ImGui::Begin("General Settings");
	static int selectedInstanceCount = scene.config.instanceCount;
	ImGui::InputInt("Count", &selectedInstanceCount, 100, 1000);
	if (ImGui::Button("Apply"))
	{
		if (selectedInstanceCount < 0) selectedInstanceCount = 0;

		if (selectedInstanceCount > static_cast<int>(scene.getMaxInstanceCount()))
			selectedInstanceCount = scene.getMaxInstanceCount();

		scene.config.instanceCount = static_cast<uint32_t>(selectedInstanceCount);
		scene.updateSpiralPositions(0.0f, false); // reset positions
		renderer.setUseGPULODCompute(false);
		renderer.setUseGPUSpiralCompute(false);
	}

	ImGui::Separator();

	ImGui::Text("LOD Settings");
	ImGui::Checkbox("Enable LOD", &scene.config.enableLOD);

	ImGui::Separator();

	if (scene.config.enableLOD)
	{
		ImGui::SliderFloat("LOD0 Range", &scene.config.lodDist0, 10.0f,				   scene.config.lodDist1);
		ImGui::SliderFloat("LOD1 Range", &scene.config.lodDist1, scene.config.lodDist0, scene.config.lodDist2);
		ImGui::SliderFloat("LOD2 Range", &scene.config.lodDist2, scene.config.lodDist1, scene.config.lodDist3);
		ImGui::SliderFloat("LOD3 Range", &scene.config.lodDist3, scene.config.lodDist2, 20000.0f);
	}

	ImGui::Separator();

	ImGui::Text("LOD Simplification");

	ImGui::SliderFloat("LOD0 % Faces", &scene.config.lodPercentageSimplification0, scene.config.lodPercentageSimplification1, 1.0f, "%.2f");
	ImGui::SliderFloat("LOD1 % Faces", &scene.config.lodPercentageSimplification1, scene.config.lodPercentageSimplification2, scene.config.lodPercentageSimplification0, "%.2f");
	ImGui::SliderFloat("LOD2 % Faces", &scene.config.lodPercentageSimplification2, scene.config.lodPercentageSimplification3, scene.config.lodPercentageSimplification1, "%.2f");
	ImGui::SliderFloat("LOD3 % Faces", &scene.config.lodPercentageSimplification3, 0.01f,									   scene.config.lodPercentageSimplification2, "%.2f");

	if (ImGui::Button("Apply and regenerate meshes"))
	{
		scene.rebuildLODs(uiCmdManager);
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

	ImGui::End();
}

const char* UserInterface::getAlgName(Algorithm algorithm)
{
	switch (algorithm)
	{
	case Algorithm::QEM: return "Edge collapse QEM";
	case Algorithm::Naive: return "Naive shortest edge collapse";
	case Algorithm::FloatingCellClustering: return "Floating cell clustering";
	case Algorithm::VertexClustering: return "Vertex clustering";
	case Algorithm::VertexDecimation: return "Vertex decimation";
	default: return "Other";
	}
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