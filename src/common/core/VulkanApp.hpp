/**
 * @author Jiri Pocarovsky (xpocar01@stud.fit.vutbr.cz)
 * @file VulkanApp.hpp
 * @brief Base class for Vulkan applications.
 *
 * This file implements the VulkanApp class for managing the Vulkan apps,
 * including initialization, main loop, and cleanup. Common interface is
 * provided to implement specific application logic further.
 */

#pragma once
#include <vulkan/vulkan.hpp>
#include <memory>
#include <chrono>

#include "core/Instance.hpp"
#include "core/Device.hpp"
#include "core/Swapchain.hpp"
#include "core/Pipeline.hpp"
#include "rendering/RenderPass.hpp"
#include "rendering/FrameBuffer.hpp"
#include "rendering/CommandManager.hpp"
#include "rendering/Renderer.hpp"
#include "rendering/Descriptors.hpp"
#include "rendering/UniformBuffer.hpp"
#include "resources/Model.hpp"
#include "resources/DualModel.hpp"
#include "scene/Camera.hpp"
#include "scene/Transform.hpp"
#include "ui/ui.hpp"
#include "window.h"

class VulkanApp {
public:
	VulkanApp(int width, int height, const char* name);
	virtual ~VulkanApp() = default;

	void run();
protected:
	virtual void init() = 0;
	virtual void update(float deltaTime) = 0;
	virtual void drawFrame() = 0;

    // RAII setup, do NOT change the order
	Window window;
    Instance instance;
    vk::UniqueSurfaceKHR surface;
    Device device;
    Swapchain swapchain;
    RenderPass renderPass;
    CommandManager commandManager;
    FrameBuffer framebuffer;
    UniformBuffer uniformBuffer;
    Descriptor descriptor;
    UserInterface ui;
    Camera camera;

    bool cameraActive = false;
	bool showUI = true;
};

/* End of the VulkanApp.hpp file */