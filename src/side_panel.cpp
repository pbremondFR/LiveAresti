#include <imgui.h>
#include <LiveAresti.hpp>

void side_panel(ImVec2 size)
{
	ImGui::BeginChild("side_panel", size, ImGuiChildFlags_Borders);
	ImGui::Button("Prev: Elite_DOUILLARD_TOMMY.seq", {-1, 0});
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("TODO: Program data preview here");
	ImGui::Button("Next: Elite_LOVICOURT_LOIC.seq", {-1, 0});
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("TODO: Program data preview here");

	ImGui::Text("ELITE_BRAGEOT_MIKA.seq");
	static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit
		| ImGuiTableFlags_RowBg
		| ImGuiTableFlags_BordersInnerV
		| ImGuiTableFlags_BordersOuter;
	if (ImGui::BeginTable("program_info", 2, table_flags))
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
	ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(200, 200, 200, 255));
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
	if (ImGui::BeginChild("chrono", {0, 0}, ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY))
	{
		double time = GetTime();
		double minutes = floor(time / 60.0);
		double seconds = fmod(time, 60.0);
		ImGui::PushFont(g_digital_font, 40);
		// Font is mono, take advantage of this to precompute text length
		float text_width = ImGui::CalcTextSize("00:00.000").x;
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_width) / 2);
		ImGui::Text("%02.0f:%06.3f", minutes, seconds);
		ImGui::PopFont();
		float button_width = ImGui::GetContentRegionAvail().x / 2 - ImGui::GetStyle().ItemInnerSpacing.x;
		ImGui::Button("Start", {button_width, 0});
		ImGui::SameLine();
		ImGui::Button("Stop", {button_width, 0});
		ImGui::EndChild();
	}
	ImGui::PopStyleColor(2);
	ImGui::EndChild();
}
