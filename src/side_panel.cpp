#include <imgui.h>
#include <LiveAresti.hpp>

static void chronometer()
{
	ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(200, 200, 200, 255));
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
	if (ImGui::BeginChild("chrono", {0, 0}, ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY))
	{
		const double time = GetTime();
		const double minutes = floor(time / 60.0);
		const double seconds = fmod(time, 60.0);
		ImGui::PushFont(g_digital_font, 40);
		// Font is mono, take advantage of this to precompute text length
		const float text_width = ImGui::CalcTextSize("00:00.000").x;
		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_width) / 2);
		ImGui::Text("%02.0f:%06.3f", minutes, seconds);
		ImGui::PopFont();
		const float button_width = ImGui::GetContentRegionAvail().x / 2 - ImGui::GetStyle().ItemInnerSpacing.x;
		ImGui::Button("Start", {button_width, 0});
		ImGui::SameLine();
		ImGui::Button("Stop", {button_width, 0});
		ImGui::EndChild();
	}
	ImGui::PopStyleColor(2);
	ImGui::EndChild();
}

static void sequence_info_table(SequenceInfo const& sequence_info)
{
	static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit
		| ImGuiTableFlags_RowBg
		| ImGuiTableFlags_BordersInnerV
		| ImGuiTableFlags_BordersOuter;
	if (ImGui::BeginTable("program_info", 2, table_flags))
	{
		ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
		ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted("File");
		ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(sequence_info.file_name.c_str());
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
}

/// Button that previews a sequence's information when it's hovered.
static bool sequence_preview_button(const char* label, ImVec2 size, SequenceInfo const* sequence_info)
{
	ImGui::BeginDisabled(sequence_info == nullptr);
	const bool clicked = ImGui::Button(label, size);
	if (sequence_info && ImGui::BeginItemTooltip())
	{
		sequence_info_table(*sequence_info);
		ImGui::EndTooltip();
	}
	ImGui::EndDisabled();
	return clicked;
}

void side_panel(ImVec2 size)
{
	ImGui::BeginChild("side_panel", size, ImGuiChildFlags_Borders);

	if (ImGui::Button("Current sequence list", {-1, 0}))
		ImGui::OpenPopup("Current sequence list");
	current_sequence_list_modal();

	/*** BEGIN/NEXT BUTTONS ***/
	const bool previous_idx_valid = g_state.current_sequence_idx - 1u < g_state.sequence_list.size();
	const SequenceData* previous_sequence = previous_idx_valid ? &g_state.sequence_list[g_state.current_sequence_idx - 1u] : nullptr;

	const bool next_idx_valid = g_state.current_sequence_idx + 1u < g_state.sequence_list.size();
	const SequenceData* next_sequence = next_idx_valid ? &g_state.sequence_list[g_state.current_sequence_idx + 1u] : nullptr;

	const float button_width = ImGui::GetContentRegionAvail().x / 2 - ImGui::GetStyle().ItemInnerSpacing.x;
	if (sequence_preview_button("Prev", {button_width, 0}, &previous_sequence->info))
	{
		g_state.current_sequence_idx--;
	}
	ImGui::SameLine();
	if (sequence_preview_button("Next", {button_width, 0}, &next_sequence->info))
	{
		g_state.current_sequence_idx++;
	}

	ImGui::Separator();

	/*** SEQUENCE INFORMATION TABLE ***/
	SequenceInfo const& sequence_info = g_state.current_sequence_idx < g_state.sequence_list.size()
		? g_state.sequence_list[g_state.current_sequence_idx].info
		: SequenceInfo();
	sequence_info_table(sequence_info);

	/*** CHRONOMETER ***/
	chronometer();
}
