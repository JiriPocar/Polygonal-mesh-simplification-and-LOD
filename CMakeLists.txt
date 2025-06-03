cmake_minimum_required(VERSION 3.10.2)

set(APP_NAME 08-helloTriangle)

project(${APP_NAME})

set(APP_SOURCES
    "src/main.cpp"
   )

set(APP_INCLUDES
   )

set(APP_SHADERS
    "src/shaders/shader.vert"
    "src/shaders/shader.frag"
   )

# dependencies
set(CMAKE_MODULE_PATH "${${APP_NAME}_SOURCE_DIR}/;${CMAKE_MODULE_PATH}")
find_package(Vulkan REQUIRED)

# executable
add_shaders("${APP_SHADERS}" APP_SHADER_DEPS)
add_executable(${APP_NAME} ${APP_SOURCES} ${APP_INCLUDES} ${APP_SHADER_DEPS})

# target
target_include_directories(${APP_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${APP_NAME} Vulkan::Vulkan)
set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 17)
