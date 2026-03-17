set(APP_NAME Spiral)

add_executable(${APP_NAME} main.cpp SpiralApp.cpp)

# LINK ENGINE
target_link_libraries(${APP_NAME} PRIVATE BPEngine)

# SHADER COMPILATION SETUP
set(LOCAL_SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(VERT_SRC "${LOCAL_SHADER_DIR}/shader.vert")
set(FRAG_SRC "${LOCAL_SHADER_DIR}/shader.frag")
set(COMP_SRC "${LOCAL_SHADER_DIR}/shader.comp")
set(SHADER_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
set(VERT_SPV "${SHADER_OUT_DIR}/shader.vert.spv")
set(FRAG_SPV "${SHADER_OUT_DIR}/shader.frag.spv")
set(COMP_SPV "${SHADER_OUT_DIR}/shader.comp.spv")

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

    add_custom_command(
        OUTPUT ${COMP_SPV}
        COMMAND ${GLSL_VALIDATOR} -V ${COMP_SRC} -o ${COMP_SPV}
        DEPENDS ${COMP_SRC}
        COMMENT "Compiling ${APP_NAME} Compute Shader"
    )

    add_custom_target(${APP_NAME}_Shaders ALL DEPENDS ${VERT_SPV} ${FRAG_SPV} ${COMP_SPV})
    add_dependencies(${APP_NAME} ${APP_NAME}_Shaders)
else()
    message(WARNING "GlslangValidator not found! Shaders will not be compiled.")
endif()

# SHADERS COMPILATION
#add_dependencies(${APP_NAME} CompileShaders)

# COPY ASSETS INTO OUTPUT DIRECTORY
add_custom_command(TARGET ${APP_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "${CMAKE_CURRENT_BINARY_DIR}/assets"
    COMMENT "Copying assets to output directory"
)