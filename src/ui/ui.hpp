/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file ui.hpp
 * @brief User interface integration via ImGui.
 * 
 * This file contains the UserInterface class which manages the integration of ImGui
*/

#pragma once
#include <vulkan/vulkan.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <deque>

#include "../window.h"
#include "../core/Device.hpp"
#include "../core/Instance.hpp"
#include "../core/Swapchain.hpp"
#include "../rendering/Renderpass.hpp"
#include "../rendering/CommandManager.hpp"
#include "../scene/Camera.hpp"
#include "../resources/Model.hpp"

class Renderer;

class UserInterface {
public:
	UserInterface(Instance& instance, Device& dev, Swapchain& swapchain, RenderPass& renderPass, Window& window, CommandManager& cmdManager);
	~UserInterface();

	void init();
	void beginFrame(std::unique_ptr<Model>& currentModel, Device& device, Renderer& renderer);
	void render(vk::CommandBuffer cmdBuffer);
	void handleMouseMove(double x, double y);

private:
	void createDescriptorPool();
	void cleanUp();

	std::deque<float> frameTimes;

	std::vector<std::string> menuModels;
	void scanModels();
	void showModelMenu(std::unique_ptr<Model>& currentModel, Device& devices, Renderer& renderer);
	void showStatistics();

	Instance& uiInstance;
	Device& uiDevice;
	Swapchain& uiSwapchain;
	RenderPass& uiRenderPass;
	Window& uiWindow;
	CommandManager& uiCmdManager;
	vk::UniqueDescriptorPool descriptorPool;
};

/* End of the ui.hpp file */