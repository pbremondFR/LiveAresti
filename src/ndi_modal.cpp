#include <imgui.h>

#include "LiveAresti.hpp"

bool NDI_modal()
{
	static char source_name[64] = {};
	static char group_name[64] = {};
	bool changed = false;
	
	if (g_state.request_open_ndi_modal)
	{
		ImGui::OpenPopup("NDI configuration");
		g_state.request_open_ndi_modal = false;
		strcpy_s(source_name, g_state.NDI_name.c_str());
		strcpy_s(group_name, g_state.NDI_group.c_str());
	}
	
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, {0.5, 0.5});
	if (ImGui::BeginPopupModal("NDI configuration", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{		
		ImGui::InputText("Source", source_name, sizeof(source_name));
		ImGui::InputText("Group", group_name, sizeof(group_name));
		if (ImGui::Button("Save"))
		{
			g_state.NDI_name = source_name;
			g_state.NDI_group = group_name;
			changed = true;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	
	return changed;
}