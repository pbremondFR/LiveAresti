#include <imgui.h>

#include "LiveAresti.hpp"

void NDI_panel(ImVec2 size)
{
	ImU32 background_color = g_state.NDI_send_ptr == nullptr
	? IM_COL32(127, 0, 0, 255)
	: IM_COL32(0, 127, 0, 255);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, background_color);
	ImGui::BeginChild("ndi_panel", size, ImGuiChildFlags_Borders);
	static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit;
	if (ImGui::BeginTable("ndi_props", 2, table_flags))
	{
		ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
			
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::Text("Name");
		ImGui::TableSetColumnIndex(1); ImGui::Text("%s", g_state.NDI_name.c_str());
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::Text("Group");
		ImGui::TableSetColumnIndex(1); ImGui::Text("%s", g_state.NDI_group.c_str());
			
		ImGui::EndTable();
	}
	ImGui::EndChild();
	ImGui::PopStyleColor();
}
