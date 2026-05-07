#include <filesystem>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <LiveAresti.hpp>
#include <string>

#include "DirectoryInputText.hpp"
#include "pugixml.hpp"

namespace fs = std::filesystem;

static std::vector<SequenceInfo> get_sequences_from_path(fs::path const& path)
{
	std::vector<SequenceInfo> sequences;

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
		sequences.emplace_back(SequenceInfo{
			.file_name = reinterpret_cast<const char*>(entry.path().stem().u8string().c_str()),
			.pilot_name = seq.child("pilot").text().as_string("???"),
			.aircraft_type = seq.child("actype").text().as_string("???"),
			.aircraft_reg = seq.child("acreg").text().as_string("???"),
			.category = seq.child("category").text().as_string("???"),
			.program = seq.child("program").text().as_string("???"),
			.sequence_text = seq.child("sequence_text").text().as_string(""),
		});
	}
	// NRVO inshallah
	return sequences;
}

void sequence_list_modal()
{
	if (g_state.request_open_sequences_modal)
	{
		ImGui::OpenPopup("Sequence list");
		g_state.request_open_sequences_modal = false;
	}
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	// Do a manual check for the open popup here instead of using ImGuiCond_Appearing to allow for the dynamic resizing
	// of the window. Otherwise position is never updated when the window size changes (due to the table filling).
	if (ImGui::IsPopupOpen("Sequence list"))
		ImGui::SetNextWindowPos(center, 0, {0.5, 0.5});

	if (ImGui::BeginPopupModal("Sequence list", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		static widget::DirectoryInputText sequences_path_widget;
		static std::vector<SequenceInfo> sequences_in_dir;

		// Don't want the length of the path selector to be too short
		ImGui::SetNextItemWidth(500);
		const bool changed = sequences_path_widget("Sequences path", "e.g. C:\\Documents\\OpenAeroSequences",
			ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_ElideLeft);
		if (changed)
		{
			sequences_in_dir = get_sequences_from_path(sequences_path_widget.get_path());
		}

		ImGui::Separator();

		static constexpr ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("Sequence list", 7, table_flags, {0, 400}))
		{
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
				SequenceInfo const& seq = sequences_in_dir[i];

				ImGui::TableNextRow();
				ImGui::PushID(seq.file_name.c_str());
				ImGui::TableSetColumnIndex(0); ImGui::Text("%d", i);
				ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(seq.file_name.c_str());
				ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(seq.pilot_name.c_str());
				ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(seq.program.c_str());
				ImGui::TableSetColumnIndex(4); ImGui::TextUnformatted(seq.category.c_str());
				ImGui::TableSetColumnIndex(5); ImGui::TextUnformatted(seq.aircraft_type.c_str());
				ImGui::TableSetColumnIndex(6); ImGui::Selectable(seq.aircraft_reg.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_DontClosePopups);

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
