cmake_minimum_required(VERSION 3.10.2)
project(BP)

# PATH CONFIG
if(DEFINED ENV{VULKAN_SDK})
    set(VULKAN_SDK_PATH "$ENV{VULKAN_SDK}")
    message(STATUS "Vulkan SDK found at: ${VULKAN_SDK_PATH}")
else()
    set(VULKAN_SDK_PATH "C:/VulkanSDK/1.4.313.0") 
endif()
set(GLSL_VALIDATOR "${VULKAN_SDK_PATH}/Bin/glslangValidator.exe" CACHE INTERNAL "Path to glslangValidator")

# DEPENDENCIES
find_package(Vulkan REQUIRED)

# SOURCE FILES
set(ENGINE_SOURCES
    "src/common/window.cpp"
    "src/common/window.h"

    # Core
    "src/common/core/Device.cpp"
    "src/common/core/Instance.cpp"
    "src/common/core/Pipeline.cpp"
    "src/common/core/SpiralComputePipeline.cpp"
    "src/common/core/SpiralPipeline.cpp"
    "src/common/core/Swapchain.cpp"
    "src/common/core/VulkanApp.cpp"

    # Rendering
    "src/common/rendering/Renderer.cpp"
    "src/common/rendering/SpiralRenderer.cpp"
    "src/common/rendering/RenderPass.cpp"
    "src/common/rendering/Descriptors.cpp"
    "src/common/rendering/CommandManager.cpp"
    "src/common/rendering/FrameBuffer.cpp"
    "src/common/rendering/UniformBuffer.cpp"

    # Resources
    "src/common/resources/Buffer.cpp"
    "src/common/resources/Model.cpp"
    "src/common/resources/DualModel.cpp"
    "src/common/resources/Textures.cpp"
    "src/common/resources/VmaUsage.cpp"

    # Scene
    "src/common/scene/Camera.cpp"
    "src/common/scene/Transform.cpp"
    "src/common/scene/Scene.cpp"
    "src/common/scene/SpiralScene.cpp"

    # Simplification
    "src/common/simplification/Simplificator.cpp"
    "src/common/simplification/Simplificator.cpp"
    "src/common/simplification/utils/Topology.cpp"
    "src/common/simplification/utils/Geometry.cpp"
    "src/common/simplification/utils/SurfApprox.cpp"
    "src/common/simplification/algorithms/QEM.cpp"
    "src/common/simplification/algorithms/VertexClustering.cpp"
    "src/common/simplification/algorithms/FloatingCellClustering.cpp"
    "src/common/simplification/algorithms/VertexDecimation.cpp"
    "src/common/simplification/algorithms/Naive.cpp"

    # UI
    "src/common/ui/ui.cpp"

    # ImGui
    "external/imgui/imgui.cpp"
    "external/imgui/imgui_draw.cpp"
    "external/imgui/imgui_tables.cpp"
    "external/imgui/imgui_widgets.cpp"
    "external/imgui/backends/imgui_impl_glfw.cpp"
    "external/imgui/backends/imgui_impl_vulkan.cpp"
)

# CREATE STATIC LIBRARY FOR ENGINE
add_library(BPEngine STATIC ${ENGINE_SOURCES})

target_compile_features(BPEngine PUBLIC cxx_std_17)

# COMMON INCLUDE DIRECTORIES
target_include_directories(BPEngine PUBLIC 
    src/common
    external
    external/glfw/include
    external/tinygltf
    external/imgui
    external/imgui/backends
    external/vma
)

# LIBRARY LINKING
target_link_directories(BPEngine PUBLIC external/glfw/lib)
target_link_libraries(BPEngine PUBLIC Vulkan::Vulkan glfw3.lib)

# C++ STANDARD
set_target_properties(BPEngine PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# DEMO APPLICATIONS
add_subdirectory(src/apps/simplificator)
add_subdirectory(src/apps/demo)