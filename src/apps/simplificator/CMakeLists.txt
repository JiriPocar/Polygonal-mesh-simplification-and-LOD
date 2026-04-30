set(APP_NAME Simplificator)

add_executable(${APP_NAME} main.cpp SimplificatorApp.cpp)

# LINK ENGINE
target_link_libraries(${APP_NAME} PRIVATE BPEngine)

# SHADER COMPILATION SETUP
set(LOCAL_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(VERT_SRC "${LOCAL_SHADER_DIR}/shader.vert")
set(FRAG_SRC "${LOCAL_SHADER_DIR}/shader.frag")
set(SHADER_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
set(VERT_SPV "${SHADER_OUT_DIR}/shader.vert.spv")
set(FRAG_SPV "${SHADER_OUT_DIR}/shader.frag.spv")

file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

if(EXISTS "${GLSL_VALIDATOR}")
    add_custom_command(
        OUTPUT ${VERT_SPV}
        COMMAND ${GLSL_VALIDATOR} -V ${VERT_SRC} -o ${VERT_SPV}
        DEPENDS ${VERT_SRC}
        COMMENT "Compiling ${APP_NAME} Vertex Shader"
    )

    add_custom_command(
        OUTPUT ${FRAG_SPV}
        COMMAND ${GLSL_VALIDATOR} -V ${FRAG_SRC} -o ${FRAG_SPV}
        DEPENDS ${FRAG_SRC}
        COMMENT "Compiling ${APP_NAME} Fragment Shader"
    )

    add_custom_target(${APP_NAME}_Shaders ALL DEPENDS ${VERT_SPV} ${FRAG_SPV})
    add_dependencies(${APP_NAME} ${APP_NAME}_Shaders)
else()
    message(WARNING "GlslangValidator not found! Shaders will not be compiled.")
endif()

# COPY ASSETS INTO OUTPUT DIRECTORY
add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "${CMAKE_CURRENT_BINARY_DIR}/assets"
    COMMENT "Copying assets to output directory"
)