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

#include "common/core/Instance.hpp"
#include "common/core/Device.hpp"
#include "common/core/Swapchain.hpp"
#include "common/core/Pipeline.hpp"
#include "common/rendering/RenderPass.hpp"
#include "common/rendering/FrameBuffer.hpp"
#include "common/rendering/CommandManager.hpp"
#include "common/rendering/Renderer.hpp"
#include "common/rendering/Descriptors.hpp"
#include "common/rendering/UniformBuffer.hpp"
#include "common/resources/Model.hpp"
#include "common/resources/DualModel.hpp"
#include "common/scene/Camera.hpp"
#include "common/scene/Transform.hpp"
#include "common/ui/ui.hpp"
#include "common/window.h"

class VulkanApp {
public:
	VulkanApp(int width, int height, bool enableVsync, const char* name);
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