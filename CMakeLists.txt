cmake_minimum_required(VERSION 3.14)
project(LiveAresti CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG master
)
# Turn off raylib examples building because we don't care
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(raylib)

# Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

# rlImGui
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG main
)
FetchContent_MakeAvailable(rlimgui)

#  raylib-extras/extras-c
FetchContent_Declare(
        raylib_extras_c
        GIT_REPOSITORY https://github.com/raylib-extras/extras-c.git
        GIT_TAG main
)
FetchContent_MakeAvailable(raylib_extras_c)

# ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)

# rlImGui sources
set(RLIMGUI_SOURCES
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/embed_roboto_medium.cpp
    src/imgui_style.cpp
    ${IMGUI_SOURCES}
    ${RLIMGUI_SOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
    ${raylib_extras_c_SOURCE_DIR}
    include
)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
