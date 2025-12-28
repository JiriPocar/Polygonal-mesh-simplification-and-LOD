set(APP_NAME DemoApp)

add_executable(${APP_NAME} main.cpp)

# LINK ENGINE
target_link_libraries(${APP_NAME} PRIVATE BPEngine)

# SHADERS COMPILATION
add_dependencies(${APP_NAME} CompileShaders)

# SHADER FILES PATHS
add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders
    COMMAND ${CMAKE_COMMAND} -E copy ${VERTEX_SPV} ${CMAKE_CURRENT_BINARY_DIR}/shaders/
    COMMAND ${CMAKE_COMMAND} -E copy ${FRAGMENT_SPV} ${CMAKE_CURRENT_BINARY_DIR}/shaders/
)