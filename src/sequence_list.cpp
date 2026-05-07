#include <filesystem>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <LiveAresti.hpp>
#include <string>

#include "DirectoryInputText.hpp"
#include "pugixml.hpp"

namespace fs = std::filesystem;

// TODO: Load textures from cached/already exported directory, for now nothing
static std::vector<SequenceData> get_sequences_from_path(fs::path const& path)
{
	std::vector<SequenceData> sequences;

	if (!fs::is_directory(path))
		return sequences;
	for (fs::directory_entry const& entry : fs::directory_iterator(path))
	{
		if (!entry.exists() || !entry.is_regular_file() || entry.path().extension().string() != ".seq")
			continue;

		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(entry.path().string().c_str());
		if (!result || doc.child("sequence").empty())
			continue;
		pugi::xml_node seq = doc.child("sequence");
		sequences.emplace_back(SequenceData{
			.info = SequenceInfo{
				.file_name = reinterpret_cast<const char*>(entry.path().stem().u8string().c_str()),
				.pilot_name = seq.child("pilot").text().as_string("???"),
				.aircraft_type = seq.child("actype").text().as_string("???"),
				.aircraft_reg = seq.child("acreg").text().as_string("???"),
				.category = seq.child("category").text().as_string("???"),
				.program = seq.child("program").text().as_string("???"),
				.sequence_text = seq.child("sequence_text").text().as_string(""),
			},
			.figures = {},
		});
	}
	// NRVO inshallah
	return sequences;
}

static bool sequence_load_button(bool loaded)
{
	if (loaded)
	{
		ImGui::BeginDisabled();
		ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 200, 0, 255));
	}
	bool clicked = ImGui::SmallButton("Load");
	if (loaded)
	{
		ImGui::PopStyleColor();
		ImGui::EndDisabled();
	}
	return clicked;
}

// TODO: Change the design. There should be a manual reload from the path.
// Add a button to load images for all programs
// Add a progress bar for loading (long time if script needs to be ran!!!). Other thread?
// Remove the shitty confusion between WIP sequence list and active sequence list (other window to reorder active
// list + select active program?)
void sequence_list_modal()
{
	bool rescan_files = false;
	if (g_state.request_open_sequences_modal)
	{
		ImGui::OpenPopup("Sequence list");
		g_state.request_open_sequences_modal = false;
		rescan_files = true;
	}
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	// Do a manual check for the open popup here instead of using ImGuiCond_Appearing to allow for the dynamic resizing
	// of the window. Otherwise position is never updated when the window size changes (due to the table filling).
	if (ImGui::IsPopupOpen("Sequence list"))
		ImGui::SetNextWindowPos(center, 0, {0.5, 0.5});

	if (ImGui::BeginPopupModal("Sequence list", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		static widget::DirectoryInputText sequences_path_widget;
		static std::vector<SequenceData> sequences_in_dir;

		// Don't want the length of the path selector to be too short
		ImGui::SetNextItemWidth(500);
		rescan_files = rescan_files || sequences_path_widget("Sequences path", "e.g. C:\\Documents\\OpenAeroSequences",
			ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_ElideLeft);
		if (rescan_files)
		{
			sequences_in_dir = get_sequences_from_path(sequences_path_widget.get_path());
		}

		ImGui::Separator();

		static constexpr ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("Sequence list", 8, table_flags, {0, 400}))
		{
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("File name", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Pilot", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Program", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Category", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Aircraft type", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Aircraft reg", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();
            ImGui::PushItemFlag(ImGuiItemFlags_AllowDuplicateId, true);
			for (int i = 0; i < sequences_in_dir.size(); ++i)
			{
				SequenceInfo const& seq = sequences_in_dir[i].info;
				const bool selected = i == g_state.current_sequence_idx;

				ImGui::TableNextRow();
				ImGui::PushID(seq.file_name.c_str());
				ImGui::TableSetColumnIndex(0); sequence_load_button(i % 2);
				ImGui::TableSetColumnIndex(1); ImGui::Text("%d", i);
				ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(seq.file_name.c_str());
				ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(seq.pilot_name.c_str());
				ImGui::TableSetColumnIndex(4); ImGui::TextUnformatted(seq.program.c_str());
				ImGui::TableSetColumnIndex(5); ImGui::TextUnformatted(seq.category.c_str());
				ImGui::TableSetColumnIndex(6); ImGui::TextUnformatted(seq.aircraft_type.c_str());
				ImGui::TableSetColumnIndex(7);
				const bool selectable_clicked = ImGui::Selectable(seq.aircraft_reg.c_str(), selected,
					ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick);
				// TODO: Probably shit design to have index selection in here (potential mismatch between files shown here
				// and files in the global state if they've changed in the meantime). Just for testing.
				if (selectable_clicked && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					g_state.current_sequence_idx = i;
				}

				ImGui::PopID();
				if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
				{
					int n_next = i + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
					if (n_next >= 0 && n_next < sequences_in_dir.size())
					{
						std::swap(sequences_in_dir[i], sequences_in_dir[n_next]);
						ImGui::ResetMouseDragDelta();
					}
				}
			}
            ImGui::PopItemFlag();
			ImGui::EndTable();
		}

		ImGui::Separator();

		if (ImGui::Button("Save"))
		{
			g_state.sequence_list = sequences_in_dir;
			g_state.sequences_dir = sequences_path_widget.get_path();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			sequences_path_widget.set_path(g_state.sequences_dir);
			sequences_in_dir = get_sequences_from_path(g_state.sequences_dir);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
