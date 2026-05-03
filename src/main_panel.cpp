#include <imgui.h>

#include "LiveAresti.hpp"

void main_panel()
{
	ImGui::BeginChild("main_controls", {0, 0}, ImGuiChildFlags_Borders);
	ImGui::Button("<< FORM B"); ImGui::SameLine(); ImGui::Button("BREAK");
	ImGui::Button("<< PREV"); ImGui::SameLine(); ImGui::Button("NEXT >>");
	{
		ImGui::BeginChild("scrolling", {0, 0}, ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_HorizontalScrollbar);
		for (int i = 0; i < 99; ++i)
		{
			ImGui::PushID(i);
			ImGui::ImageButton(std::to_string(i).c_str(), ImTextureID(g_state.test_texture.id), {100, 100});
			ImGui::SameLine();
			ImGui::PopID();
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
}