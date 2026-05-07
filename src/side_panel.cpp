#include <imgui.h>
#include <LiveAresti.hpp>

void side_panel(ImVec2 size)
{
	ImGui::BeginChild("side_panel", size, ImGuiChildFlags_Borders);

	/*** BEGIN/NEXT BUTTONS ***/
	const bool previous_idx_valid = g_state.current_sequence_idx - 1u < g_state.sequence_list.size();
	const std::string previous_file = std::format("Prev: {}",
		previous_idx_valid ? g_state.sequence_list[g_state.current_sequence_idx - 1u].info.file_name : "N/A");
	const bool next_idx_valid = g_state.current_sequence_idx + 1u < g_state.sequence_list.size();
	const std::string next_file = std::format("Next: {}",
		next_idx_valid ? g_state.sequence_list[g_state.current_sequence_idx + 1u].info.file_name : "N/A");

	ImGui::BeginDisabled(!previous_idx_valid);
	if (ImGui::Button(previous_file.c_str(), {-1, 0}))
		g_state.current_sequence_idx--;
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("TODO: Program data preview here");
	ImGui::EndDisabled();

	ImGui::BeginDisabled(!next_idx_valid);
	if (ImGui::Button(next_file.c_str(), {-1, 0}))
		g_state.current_sequence_idx++;
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("TODO: Program data preview here");
	ImGui::EndDisabled();

	/*** SEQUENCE INFORMATION TABLE ***/
	SequenceInfo const& sequence_info = g_state.current_sequence_idx < g_state.sequence_list.size()
		? g_state.sequence_list[g_state.current_sequence_idx].info
		: SequenceInfo();
	ImGui::TextUnformatted(sequence_info.file_name.c_str());
	static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit
		| ImGuiTableFlags_RowBg
		| ImGuiTableFlags_BordersInnerV
		| ImGuiTableFlags_BordersOuter;
	if (ImGui::BeginTable("program_info", 2, table_flags))
	{
		ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
		
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Program");
		ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(sequence_info.program.c_str());
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Pilot");
		ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(sequence_info.pilot_name.c_str());
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Aircraft type");
		ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(sequence_info.aircraft_type.c_str());
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Aircraft reg");
		ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(sequence_info.aircraft_reg.c_str());
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("Category");
		ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(sequence_info.category.c_str());
		
		ImGui::EndTable();
	}

	/*** CHRONOMETER ***/
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
