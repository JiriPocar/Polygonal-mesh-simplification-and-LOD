cmake_minimum_required(VERSION 3.10.2)

set(APP_NAME BP)
project(${APP_NAME})

set(APP_SOURCES
    "src/main.cpp"
    "src/window.cpp"
    "src/window.h"
)

set(APP_INCLUDES
)

set(APP_SHADERS
    "src/shaders/shader.vert"
    "src/shaders/shader.frag"
)

# Dependencies
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/;${CMAKE_MODULE_PATH}")
find_package(Vulkan REQUIRED)

# Executable
add_executable(${APP_NAME} ${APP_SOURCES} ${APP_SHADERS} "src/core/Device.cpp" "src/core/Instance.cpp" "src/core/Pipeline.cpp" "src/rendering/Renderer.cpp" "src/rendering/RenderPass.cpp" "src/rendering/Descriptors.cpp" "src/rendering/CommandManager.cpp" "src/resources/Mesh.cpp" "src/resources/Textures.cpp" "src/resources/Model.cpp" "src/resources/ResourceManager.cpp" "src/scene/Camera.cpp" "src/scene/Light.cpp" "src/scene/Scene.cpp")

# Include directories
target_include_directories(${APP_NAME} PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}
    external/glfw/include)

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