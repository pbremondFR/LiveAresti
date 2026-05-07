#include <imgui.h>
#include <LiveAresti.hpp>
#include <Processing.NDI.Lib.h>
#include <Processing.NDI.Send.h>

float imgui_menu_bar(bool &should_close, bool &show_demo_window)
{
	float menu_bar_height = 0.0f;
	if (ImGui::BeginMainMenuBar())
	{
		menu_bar_height = ImGui::GetWindowSize().y;
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Set sequences directory..."))
			{
				g_state.request_open_sequences_modal = true;
			}
			if (ImGui::MenuItem("Load raw sequence"))
				;
			ImGui::Separator();
			if (ImGui::MenuItem("Quit"))
				should_close = true;
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("NDI"))
		{
			bool broadcasting = g_state.NDI_send_ptr != nullptr;
			if (ImGui::MenuItem("Set up NDI source...", nullptr, false, !broadcasting))
			{
				g_state.request_open_ndi_modal = true;
			}
			if (ImGui::MenuItem("Broadcast", nullptr, broadcasting, true))
			{
				if (!broadcasting)
				{
					NDIlib_send_create_t NDI_send_create_desc;
					NDI_send_create_desc.p_ndi_name = g_state.NDI_name.c_str();
					NDI_send_create_desc.p_groups = g_state.NDI_group.c_str();
					NDI_send_create_desc.clock_video = true;
					NDI_send_create_desc.clock_audio = false;
					g_state.NDI_send_ptr = NDIlib_send_create(&NDI_send_create_desc);
				}
				else
				{
					NDIlib_send_destroy(g_state.NDI_send_ptr);
					g_state.NDI_send_ptr = nullptr;
				}
			}
			ImGui::EndMenu();
		}
 
		if (ImGui::BeginMenu("Window"))
		{
			if (ImGui::MenuItem("Demo Window", nullptr, show_demo_window))
				show_demo_window = !show_demo_window;
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	return menu_bar_height;
}
