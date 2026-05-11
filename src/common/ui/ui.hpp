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

#include "common/window.h"
#include "common/core/Device.hpp"
#include "common/core/Instance.hpp"
#include "common/core/Swapchain.hpp"
#include "common/rendering/Renderpass.hpp"
#include "common/rendering/CommandManager.hpp"
#include "common/scene/Camera.hpp"
#include "common/scene/Transform.hpp"
#include "common/resources/Model.hpp"
#include "common/resources/DualModel.hpp"
#include "common/simplification/Simplificator.hpp"
#include "apps/simplificator/SimplificatorRenderer.hpp"
#include "apps/spiral/SpiralRenderer.hpp"
#include "apps/spiral/Benchmark.hpp"

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
	std::vector<std::string> outModels;
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
	void showTextureControls(std::unique_ptr<DualModel>& currentDualModel);

	// spiral app interface
	void showSpiralControls(SpiralScene& scene);
	void showGeneralControls(SpiralScene& scene, SpiralRenderer& renderer);
	void showWireframeControls2(SpiralRenderer& renderer);
	void showUseGPUCPUControls(SpiralRenderer& renderer);
	void showSceneInfo(SpiralScene& scene, glm::vec3 camPos);
	void showBenchmarkStart(Benchmark& benchmark);
	void showBenchmarkStatus(Benchmark& benchmark);

	Instance& m_instance;
	Device& m_device;
	Swapchain& m_swapchain;
	RenderPass& m_renderPass;
	Window& m_window;
	CommandManager& m_cmd;

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