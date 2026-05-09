cmake_minimum_required(VERSION 3.14)
project(LiveAresti CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0
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
    GIT_TAG Raylib_6_0
)
FetchContent_MakeAvailable(rlimgui)

# raylib-extras/extras-c
FetchContent_Declare(
    raylib_extras_c
    GIT_REPOSITORY https://github.com/raylib-extras/extras-c.git
    GIT_TAG main
)
FetchContent_MakeAvailable(raylib_extras_c)

# pugixml
FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG v1.15
)
FetchContent_MakeAvailable(pugixml)

# raylib-cpp: C++ wrapper types for RayLib
FetchContent_Declare(
    raylib_cpp
    GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
    GIT_TAG v6.0.0
)
FetchContent_MakeAvailable(raylib_cpp)

FetchContent_Declare(
    ArestiExporter
    URL https://github.com/pbremondFR/ArestiExporter/releases/download/v1.0.0/ArestiExporter-win-x64.zip
    # cmake -E sha256sum arestiexporter-win-x64.zip
    URL_HASH SHA256=40a92736da8a852b7b8bfd3684bcac45c9adde4dd1db14a6df4d5c0d5b588018
)
FetchContent_MakeAvailable(ArestiExporter)
add_compile_definitions(ARESTIEXPORTER_EXE="ArestiExporter\\ArestiExporter.exe")

# ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

# rlImGui sources
set(RLIMGUI_SOURCES
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)

set(NDI_DIR $ENV{NDI_SDK_DIR})
if(NOT NDI_DIR)
    message(FATAL_ERROR "NDI SDK is required: https://ndi.video/for-developers/ndi-sdk/download/")
endif()

set(NDI_INCLUDE_DIR "${NDI_DIR}/Include")
set(NDI_LIB_DIR "${NDI_DIR}/Lib/x64")
set(NDI_BIN_DIR "${NDI_DIR}/Bin/x64")

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/embed_roboto_medium.cpp
    src/embed_dseg14_classic_italic.cpp
    src/imgui_style.cpp
    src/ndi_modal.cpp
    src/ndi_panel.cpp
    src/side_panel.cpp
    src/main_panel.cpp
    src/menu_bar.cpp
    src/ndi_export.cpp
    src/load_sequences_from_dir.cpp
    src/current_sequence_list.cpp
    ${IMGUI_SOURCES}
    ${RLIMGUI_SOURCES}
    include/LiveAresti.hpp
    include/DirectoryInputText.hpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
    ${raylib_extras_c_SOURCE_DIR}
    ${NDI_INCLUDE_DIR}
    include
)

# Link to Windows subsystem instead of Console to avoid have a cmd open at program start
# and set the entry to main() instead of WinMain() which we'd need to manually define.
# https://stackoverflow.com/questions/22934206/what-is-the-difference-between-main-and-maincrtstartup
target_link_options(${PROJECT_NAME} PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
target_link_directories(${PROJECT_NAME} PRIVATE "${NDI_LIB_DIR}")
target_link_libraries(${PROJECT_NAME} PRIVATE raylib raylib_cpp Processing.NDI.Lib.x64 pugixml)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${NDI_BIN_DIR}/Processing.NDI.Lib.x64.dll"
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# Copy the ArestiExporter release files
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${arestiexporter_SOURCE_DIR}/ArestiExporter.exe"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/ArestiExporter/ArestiExporter.exe"

    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    "${arestiexporter_SOURCE_DIR}/firefox"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/ArestiExporter/firefox"

    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    "${arestiexporter_SOURCE_DIR}/OpenAero"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/ArestiExporter/OpenAero"
)
