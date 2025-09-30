cmake_minimum_required(VERSION 3.10.2)

set(APP_NAME BP)
project(${APP_NAME})

set(APP_SOURCES
    "src/main.cpp"
    "src/window.cpp"
    "src/window.h"
)

# Set path to Vulkan SDK
set(VULKAN_SDK_PATH "C:/VulkanSDK/1.4.313.0")
set(GLSL_VALIDATOR "${VULKAN_SDK_PATH}/Bin/glslangValidator.exe")

# Check for glslangValidator
if(NOT EXISTS "${GLSL_VALIDATOR}")
    message(WARNING "glslangValidator not found at: ${GLSL_VALIDATOR}")
    message("Shader compilation will be skipped")
endif()

set(APP_INCLUDES
)

# Shader files
set(VERTEX_SHADER "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/shader.vert")
set(FRAGMENT_SHADER "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/shader.frag")

# Output SPIR-V files
set(VERTEX_SPV "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/shader.vert.spv")
set(FRAGMENT_SPV "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/shader.frag.spv")

# Shader compilation
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
    
    set(APP_SOURCES ${APP_SOURCES} ${VERTEX_SPV} ${FRAGMENT_SPV})
else()
    message(WARNING "Shader compilation disabled - glslangValidator not found")
endif()

# Dependencies
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/;${CMAKE_MODULE_PATH}")
find_package(Vulkan REQUIRED)

set(IMGUI_SOURCES
    "external/imgui/imgui.cpp"
    "external/imgui/imgui_draw.cpp"
    "external/imgui/imgui_tables.cpp"
    "external/imgui/imgui_widgets.cpp"
    "external/imgui/backends/imgui_impl_glfw.cpp"
    "external/imgui/backends/imgui_impl_vulkan.cpp"
)


# Executable
add_executable(${APP_NAME}
    ${APP_SOURCES}
    ${APP_SHADERS}
    ${IMGUI_SOURCES}
    "src/core/Device.cpp"
    "src/core/Instance.cpp"
    "src/core/Pipeline.cpp"
    "src/core/Swapchain.cpp"
    "src/rendering/Renderer.cpp"
    "src/rendering/RenderPass.cpp"
    "src/rendering/Descriptors.cpp"
    "src/rendering/CommandManager.cpp"
    "src/resources/Mesh.cpp"
    "src/resources/Textures.cpp"
    "src/resources/Model.cpp"
    
    "src/scene/Camera.cpp"
    
    "src/scene/Scene.cpp"
    "src/rendering/FrameBuffer.cpp"
    "external/tinygltf/stb_image.h"
    "external/tinygltf/stb_image_write.h"
    "external/tinygltf/tiny_gltf.h"
    "src/resources/Buffer.cpp"
    "src/rendering/UniformBuffer.cpp"
    "src/scene/Transform.cpp"
    "src/ui/ui.cpp"
 "external/glfw/include/GLFW/glfw3native.h")

if(EXISTS "${GLSL_VALIDATOR}")
    add_dependencies(${APP_NAME} CompileShaders)
endif()

# Include directories
target_include_directories(${APP_NAME} PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}
    external/glfw/include
    external/tinygltf
    external/imgui
    external/imgui/backends
)

# Link directories
target_link_directories(${APP_NAME} PRIVATE external/glfw/lib)

# Link libraries
target_link_libraries(${APP_NAME} PRIVATE 
    Vulkan::Vulkan
    glfw3.lib
)

# C++ standard
set_target_properties(${APP_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Copy shaders to output directory
add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory 
        ${CMAKE_CURRENT_BINARY_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy
        ${VERTEX_SHADER} ${CMAKE_CURRENT_BINARY_DIR}/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy
        ${FRAGMENT_SHADER} ${CMAKE_CURRENT_BINARY_DIR}/shaders/
    COMMENT "Copying shaders to output directory"
)