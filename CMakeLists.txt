cmake_minimum_required(VERSION 3.10.2)
project(BP)

# PATH CONFIG
set(VULKAN_SDK_PATH "C:/VulkanSDK/1.4.313.0")
set(GLSL_VALIDATOR "${VULKAN_SDK_PATH}/Bin/glslangValidator.exe")

# SHADERS COMPILATION
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/common/shaders")
set(VERTEX_SHADER "${SHADER_DIR}/shader.vert")
set(FRAGMENT_SHADER "${SHADER_DIR}/shader.frag")
set(VERTEX_SPV "${SHADER_DIR}/shader.vert.spv")
set(FRAGMENT_SPV "${SHADER_DIR}/shader.frag.spv")

if(EXISTS "${GLSL_VALIDATOR}")
    add_custom_command(
        OUTPUT ${VERTEX_SPV}
        COMMAND ${GLSL_VALIDATOR} -V ${VERTEX_SHADER} -o ${VERTEX_SPV}
        DEPENDS ${VERTEX_SHADER}
        COMMENT "Compiling vertex shader"
    )
    add_custom_command(
        OUTPUT ${FRAGMENT_SPV}
        COMMAND ${GLSL_VALIDATOR} -V ${FRAGMENT_SHADER} -o ${FRAGMENT_SPV}
        DEPENDS ${FRAGMENT_SHADER}
        COMMENT "Compiling fragment shader"
    )
    add_custom_target(CompileShaders ALL DEPENDS ${VERTEX_SPV} ${FRAGMENT_SPV})
endif()

# DEPENDENCIES
find_package(Vulkan REQUIRED)

# SOURCE FILES
set(ENGINE_SOURCES
    "src/common/window.cpp"
    "src/common/window.h"
    "src/common/core/Device.cpp"
    "src/common/core/Instance.cpp"
    "src/common/core/Pipeline.cpp"
    "src/common/core/Swapchain.cpp"
    "src/common/rendering/Renderer.cpp"
    "src/common/rendering/RenderPass.cpp"
    "src/common/rendering/Descriptors.cpp"
    "src/common/rendering/CommandManager.cpp"
    "src/common/rendering/FrameBuffer.cpp"
    "src/common/rendering/UniformBuffer.cpp"
    "src/common/resources/Buffer.cpp"
    "src/common/resources/Model.cpp"
    "src/common/resources/DualModel.cpp"
    "src/common/scene/Camera.cpp"
    "src/common/scene/Transform.cpp"
    "src/common/scene/Scene.cpp"
    "src/common/simplification/Simplificator.cpp"
    "src/common/simplification/simplificationUtil.cpp"
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