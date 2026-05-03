#include <imgui.h>
#include <LiveAresti.hpp>

static ImGuiStyle get_imgui_style()
{
	ImGuiStyle style = ImGuiStyle();
	ImGui::StyleColorsDark(&style);
	
	style.Colors[ImGuiCol_WindowBg] = ImColor{30, 30, 30, 255};
	style.Colors[ImGuiCol_ChildBg] = ImColor{15, 15, 15, 255};
	style.Colors[ImGuiCol_MenuBarBg] = ImColor{60, 60, 60, 255};
	
	style.FrameRounding = 3;
	// style.ItemSpacing.y = 6;
	style.GrabRounding = 4;
	style.PopupRounding = 3;
	//style.WindowRounding = 4;
	style.ChildRounding = 6;
	style.WindowBorderSize = 0;

	style.ScaleAllSizes(1.5f);
	
	return style;
}

void apply_imgui_app_style()
{
	ImGui::GetStyle() = get_imgui_style();
	
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.Fonts->Clear();
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(Roboto_Medium_compressed_data_base85, 17);
	io.Fonts->Build();
}
