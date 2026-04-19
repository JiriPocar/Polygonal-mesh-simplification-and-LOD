/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file ui.hpp
 * @brief User interface integration via ImGui.
 * 
 * This file contains the UserInterface class, which integrates ImGui into the Vulkan application.
 * Provides methods for initializing ImGui, creating a descriptor pool, and rendering the UI elements each frame.
*/

#pragma once
#include <vulkan/vulkan.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <deque>
#include <vector>

#include "../window.h"
#include "../core/Device.hpp"
#include "../core/Instance.hpp"
#include "../core/Swapchain.hpp"
#include "../rendering/Renderpass.hpp"
#include "../rendering/CommandManager.hpp"
#include "../scene/Camera.hpp"
#include "../scene/Transform.hpp"
#include "../resources/Model.hpp"
#include "../resources/DualModel.hpp"
#include "../simplification/Simplificator.hpp"
#include "../apps/simplificator/SimplificatorRenderer.hpp"
#include "../apps/demo/SpiralRenderer.hpp"
#include "../apps/demo/Benchmark.hpp"

class SpiralScene;
class SimplificatorRenderer;
class SpiralRenderer;

class UserInterface {
public:
	UserInterface(Instance& instance, Device& dev, Swapchain& swapchain, RenderPass& renderPass, Window& window, CommandManager& cmdManager);
	~UserInterface();

	void init();
	void beginFrame(std::unique_ptr<DualModel>& currentDualModel, Device& device, SimplificatorRenderer& renderer, Transform& transform, bool show);
	void beginFrame2(SpiralScene& scene, SpiralRenderer& renderer, glm::vec3 camPos, Benchmark& benchmark, bool show);
	void render(vk::CommandBuffer cmdBuffer);
	void handleMouseMove(double x, double y);

	Transform fetchTransform();
	void setTransform(const Transform& transform);
	std::vector<bool> getRotationAxes() const {
		return { rotateByX, rotateByY, rotateByZ };
	}

private:
	void createDescriptorPool();
	void cleanUp();

	std::deque<float> frameTimes;

	// simplificator app interface
	std::vector<std::string> menuModels;
	std::string selectedModel;
	void scanModels();
	void showModelMenu(std::unique_ptr<DualModel>& currentDualModel, Device& devices, SimplificatorRenderer& renderer, Transform& transform);
	void showStatistics();
	void showSimplificationControls(std::unique_ptr<DualModel>& currentDualModel, Device& device);
	void showModelPerspectiveControls(Transform& transform);
	void showWireframeControls(SimplificatorRenderer& renderer);
	void showSmoothingControls();
	void showSimplificationResults(std::unique_ptr<DualModel>& currentDualModel);
	void showSurfaceApproximationErrorControls();

	// spiral app interface
	void showSpiralControls(SpiralScene& scene);
	void showGeneralControls(SpiralScene& scene, SpiralRenderer& renderer);
	void showWireframeControls2(SpiralRenderer& renderer);
	void showUseGPUCPUControls(SpiralRenderer& renderer);
	void showSceneInfo(SpiralScene& scene, glm::vec3 camPos);
	void showBenchmarkStart(Benchmark& benchmark);
	void showBenchmarkStatus(Benchmark& benchmark);

	Instance& uiInstance;
	Device& uiDevice;
	Swapchain& uiSwapchain;
	RenderPass& uiRenderPass;
	Window& uiWindow;
	CommandManager& uiCmdManager;
	vk::UniqueDescriptorPool descriptorPool;
	Simplificator simplificator;

	Transform uiTransform;
	bool rotateByX = false;
	bool rotateByY = true;
	bool rotateByZ = false;

	SimplificatorResult lastResult;
	bool hasSimplificationResult = false;
	const char* getAlgName(Algorithm algorithm);
};

/* End of the ui.hpp file */