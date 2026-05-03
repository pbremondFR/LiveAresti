#include <imgui.h>

void side_panel(ImVec2 size)
{
	ImGui::BeginChild("side_panel", size, ImGuiChildFlags_Borders);
	ImGui::Text("ELITE_BRAGEOT_MIKA.seq");
	static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit
		| ImGuiTableFlags_RowBg
		| ImGuiTableFlags_BordersInnerV
		| ImGuiTableFlags_BordersOuter;
	if (ImGui::BeginTable("ndi_props", 2, table_flags))
	{
		ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
		
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::Text("Program");
		ImGui::TableSetColumnIndex(1); ImGui::Text("Free Known");
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::Text("Pilot");
		ImGui::TableSetColumnIndex(1); ImGui::Text("Mika BRAGEOT");
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::Text("Aircraft type");
		ImGui::TableSetColumnIndex(1); ImGui::Text("EXTRA330SC");
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::Text("Aircraft reg");
		ImGui::TableSetColumnIndex(1); ImGui::Text("F-HMKF");
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::Text("Category");
		ImGui::TableSetColumnIndex(1); ImGui::Text("Unlimited");
		
		ImGui::EndTable();
	}
	ImGui::EndChild();
}
