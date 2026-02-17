cmake_minimum_required(VERSION 3.20)
project(Argon LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- GLFW options ----
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

add_subdirectory(libraries/glfw)

# ---- Engine library ----
add_library(argon STATIC
    # renderer
    src/renderer/shader.cpp
    src/renderer/mesh.cpp
    src/renderer/renderer.cpp
    src/renderer/texture2d.cpp

    # platform
    src/platform/window.cpp

    # gfx
    src/gfx/camera_controller2d.cpp

    # glad (C)
    libraries/glad/src/glad.c

    # imgui
    libraries/imgui/imgui.cpp
    libraries/imgui/imgui_draw.cpp
    libraries/imgui/imgui_tables.cpp
    libraries/imgui/imgui_widgets.cpp
    libraries/imgui/backends/imgui_impl_glfw.cpp
    libraries/imgui/backends/imgui_impl_opengl3.cpp
    # libraries/imgui/imgui_demo.cpp 
 "src/renderer/material2d.h" "src/scene/entity.h" "src/scene/scene.cpp" "src/scene/frame_context.h" "src/systems/movement_system.cpp" "src/systems/camera_system.cpp" "src/systems/render_system2d.h" "src/systems/render_system2d.cpp" "src/renderer/material_handle.h" "src/renderer/material_library.h" "src/renderer/material_library.cpp" "src/renderer/render_packet2d.h" "src/renderer/render_frame2d.h" "src/renderer/render_pass2d.h" "src/renderer/render_pass2d.cpp" "src/renderer/render_pipeline2d.h" "src/renderer/render_pipeline2d.cpp" "src/renderer/imgui_pass2d.h" "src/renderer/imgui_pass2d.cpp")

# Expose include dirs to anything that links argon
target_include_directories(argon PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/libraries/glad/include
    ${CMAKE_SOURCE_DIR}/libraries/stb
    ${CMAKE_SOURCE_DIR}/libraries/imgui
    ${CMAKE_SOURCE_DIR}/libraries/imgui/backends

)

# Link dependencies as PUBLIC so sandbox inherits them automatically
target_link_libraries(argon PUBLIC glfw)

# On some platforms you may need additional libs (usually GLFW handles this)
# if(UNIX AND NOT APPLE)
#   target_link_libraries(argon PUBLIC dl pthread)
# endif()

# ---- Sandbox executable ----
add_executable(sandbox
    sandbox/main.cpp
    "sandbox/sandbox.cpp"
 "src/renderer/material2d.h" "src/scene/entity.h" "src/scene/scene.cpp" "src/scene/frame_context.h" "src/systems/movement_system.cpp" "src/systems/camera_system.cpp" "src/systems/render_system2d.h" "src/systems/render_system2d.cpp" "src/renderer/material_handle.h" "src/renderer/material_library.h" "src/renderer/material_library.cpp" "src/renderer/render_packet2d.h" "src/renderer/render_frame2d.h" "src/renderer/render_pass2d.h" "src/renderer/render_pass2d.cpp" "src/renderer/render_pipeline2d.h" "src/renderer/render_pipeline2d.cpp" "src/renderer/imgui_pass2d.h" "src/renderer/imgui_pass2d.cpp")

target_link_libraries(sandbox PRIVATE argon)

# Optional: set nicer output name
# set_target_properties(sandbox PROPERTIES OUTPUT_NAME "ArgonSandbox")
add_custom_command(
    TARGET sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/sandbox/assets
            $<TARGET_FILE_DIR:sandbox>/assets
)