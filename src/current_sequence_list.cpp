#include <imgui.h>

#include "LiveAresti.hpp"

void current_sequence_list_modal()
{
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, {0.5, 0.5});

	if (ImGui::BeginPopupModal("Current sequence list", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
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
			for (int i = 0; i < g_state.sequence_list.size(); ++i)
			{
				SequenceInfo const& seq = g_state.sequence_list[i].info;
				const bool selected = i == g_state.current_sequence_idx;

				ImGui::TableNextRow();
				ImGui::PushID(seq.file_name.c_str());
				ImGui::TableSetColumnIndex(0); ImGui::Text("%d", i);
				ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(seq.file_name.c_str());
				ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(seq.pilot_name.c_str());
				ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(seq.program.c_str());
				ImGui::TableSetColumnIndex(4); ImGui::TextUnformatted(seq.category.c_str());
				ImGui::TableSetColumnIndex(5); ImGui::TextUnformatted(seq.aircraft_type.c_str());
				ImGui::TableSetColumnIndex(6);
				const bool selectable_clicked = ImGui::Selectable(seq.aircraft_reg.c_str(), selected,
					ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick);
				if (selectable_clicked && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					g_state.current_sequence_idx = i;
				}

				ImGui::PopID();
				if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
				{
					int n_next = i + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
					if (n_next >= 0 && n_next < g_state.sequence_list.size())
					{
						std::swap(g_state.sequence_list[i], g_state.sequence_list[n_next]);
						ImGui::ResetMouseDragDelta();
					}
				}
			}
            ImGui::PopItemFlag();
			ImGui::EndTable();
		}

		ImGui::Separator();

		if (ImGui::Button("Close"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
