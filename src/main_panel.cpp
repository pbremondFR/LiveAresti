#include <imgui.h>

#include "LiveAresti.hpp"

void main_panel()
{
	ImGui::BeginChild("main_controls", {0, 0}, ImGuiChildFlags_Borders);
	ImGui::Button("<< FORM B"); ImGui::SameLine(); ImGui::Button("BREAK");
	ImGui::Button("<< PREV"); ImGui::SameLine(); ImGui::Button("NEXT >>");
	{
		ImGui::BeginChild("scrolling", {0, 0}, ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_HorizontalScrollbar);
		if (g_state.current_sequence_idx < g_state.sequence_list.size())
		{
			SequenceData const& sequence_data = g_state.sequence_list[g_state.current_sequence_idx];
			for (int i = 0; i < sequence_data.figures.size(); ++i)
			{
				ImGui::PushID(i);
				ImGui::ImageButton("",
					ImTextureID(sequence_data.figures[i].texture_form_b.id),
					{100, 100}
				);
				ImGui::SameLine();
				ImGui::PopID();
			}
		}
		else
		{
			ImGui::Text("No program loaded!");
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();
}
