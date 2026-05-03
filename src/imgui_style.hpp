#pragma once

#include "imgui.h"

inline void apply_app_style()
{
    auto& style = ImGui::GetStyle();
    
    style.Colors[ImGuiCol_WindowBg] = ImColor{32, 32, 32, 255};
    style.Colors[ImGuiCol_MenuBarBg] = ImColor{64, 64, 64, 255};
}