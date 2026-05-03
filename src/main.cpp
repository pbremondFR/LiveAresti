/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"
#include "LiveAresti.hpp"
#include "path_utils/resource_dir.h"
#include <Processing.NDI.Lib.h>
#include <string>

LiveArestiState g_state = {};

Texture test_texture;
RenderTexture2D test_output_target;
float menu_bar_height = 0.0f;

void imgui_menu_bar(bool &should_close, bool &show_demo_window)
{
	if (ImGui::BeginMainMenuBar())
	{
		menu_bar_height = ImGui::GetWindowSize().y;
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Load .seq"))
				;
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
}

void imgui_main_app_window()
{
	// This is the main display area, that fills the app's window
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration
		| ImGuiWindowFlags_NoBringToFrontOnFocus
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoScrollWithMouse;
	ImGui::GetStyle().WindowBorderSize = 0;
	// Disable this to avoid highlighting issues with combo search box
	ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
	
	ImGui::Begin("main", nullptr, flags);
	
	NDI_modal();
	
	const float origin_y_pos = menu_bar_height + ImGui::GetStyle().WindowPadding.y;
	const float side_panel_width = viewport->WorkSize.x - 600 - ImGui::GetStyle().WindowPadding.x*2 - ImGui::GetStyle().ItemSpacing.y; 
	const ImVec2 NDI_panel_size = {side_panel_width, 70};
	const ImVec2 side_panel_size = {side_panel_width, 600 - NDI_panel_size.y - ImGui::GetStyle().ItemSpacing.y};
	{
		ImU32 background_color = g_state.NDI_send_ptr == nullptr
			? IM_COL32(127, 0, 0, 255)
			: IM_COL32(0, 127, 0, 255);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, background_color);
		ImGui::SetNextWindowPos({ImGui::GetStyle().WindowPadding.x, origin_y_pos});
		ImGui::BeginChild("ndi_panel", NDI_panel_size, ImGuiChildFlags_Borders);
        static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit;
		if (ImGui::BeginTable("ndi_props", 2, table_flags))
		{
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
			
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::Text("Name");
			ImGui::TableSetColumnIndex(1); ImGui::Text("%s", g_state.NDI_name.c_str());
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0); ImGui::Text("Group");
			ImGui::TableSetColumnIndex(1); ImGui::Text("%s", g_state.NDI_group.c_str());
			
			ImGui::EndTable();
		}
		ImGui::EndChild();
		ImGui::PopStyleColor();
	}
	{
		ImGui::SetNextWindowPos({
			ImGui::GetStyle().WindowPadding.x,
			origin_y_pos + NDI_panel_size.y + ImGui::GetStyle().ItemSpacing.y
		});
		ImGui::BeginChild("side_panel", side_panel_size, ImGuiChildFlags_Borders);
		ImGui::Text("ELITE_BRAGEOT_MIKA.seq");
		static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuter;
		if (ImGui::BeginTable("ndi_props", 2, table_flags))
		{
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthStretch);
			
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
	{
		ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(127, 127, 127, 255));
		ImGui::SetNextWindowPos({viewport->WorkSize.x - 600 - ImGui::GetStyle().WindowPadding.x, origin_y_pos});
		ImGui::BeginChild("preview", {600, 600});
		rlImGuiImageRenderTexture(&test_output_target);
		ImGui::EndChild();
		ImGui::PopStyleColor();
	}
	

	ImGui::BeginChild("main_controls", {0, 0}, ImGuiChildFlags_Borders);
	ImGui::Text("Hello, world!");
	ImGui::Text("This is where the controls should go.");
	ImGui::Button("<< FORM B"); ImGui::SameLine(); ImGui::Button("BREAK");
	ImGui::Button("<< PREV"); ImGui::SameLine(); ImGui::Button("NEXT >>");
	{
        ImGui::BeginChild("scrolling", {0, 0}, ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_HorizontalScrollbar);
		for (int i = 0; i < 99; ++i)
		{
			// ImGui::PushID(i); ImGui::Button("<< FORM B"); ImGui::SameLine(); ImGui::PopID();
			ImGui::PushID(i);
			ImGui::ImageButton(std::to_string(i).c_str(), ImTextureID(test_texture.id), {100, 100});
			// rlImGuiImageRect(&test_texture, 100, 100, Rectangle{ 0,0, float(test_texture.width), float(test_texture.height) });
			ImGui::SameLine();
			ImGui::PopID();
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();

	ImGui::End();
}

int main()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);

	// Create the window and OpenGL context
	InitWindow(1000, 1000, "LiveAresti");
	
	rlImGuiSetup(true);
	apply_imgui_app_style();
	if (!NDIlib_initialize())
	{
		assert(false);
	}

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// Load a texture from the resources directory
	test_texture = LoadTexture("MIKA BRAGEOT_Free Known_FormB_Fig01.png");
	
	test_output_target = LoadRenderTexture(600, 600);
	
	Shader maskShader = LoadShader(nullptr, "border_mask.frag");
	
	bool should_close = false;
	bool show_demo_window = false;
	
	// game loop
	while (!WindowShouldClose() && !should_close)		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		BeginTextureMode(test_output_target);
		BeginShaderMode(maskShader);		
		ClearBackground(BLANK);
		DrawTexturePro(test_texture,
			{ 0.0f, 0.0f, (float)test_texture.width, (float)test_texture.height },
			{ 600.0f, 600.0f, (float)test_texture.width, (float)test_texture.height },
			{600, 600},
			static_cast<float>(GetTime() * 100),
			// 45,
			WHITE);
		EndShaderMode();
		EndTextureMode();
		
		Image output_image_ndi = LoadImageFromTexture(test_output_target.texture);
		ImageFormat(&output_image_ndi, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
		ImageFlipVertical(&output_image_ndi);
		NDIlib_video_frame_v2_t NDI_video_frame;
		NDI_video_frame.xres = test_output_target.texture.width;
		NDI_video_frame.yres = test_output_target.texture.height;
    
		NDI_video_frame.frame_rate_N = 60000;
		NDI_video_frame.frame_rate_D = 1000;
		NDI_video_frame.FourCC = NDIlib_FourCC_type_RGBA;
    
		NDI_video_frame.p_data = static_cast<uint8_t*>(output_image_ndi.data);
		NDI_video_frame.line_stride_in_bytes = test_output_target.texture.width * 4;

		// Optionnel mais recommandé : NDI gère le framerate
		// En appelant cette fonction, NDI va "bloquer" légèrement si vous envoyez
		// trop vite, afin de maintenir un flux fluide (ex: 60fps constants).
		NDIlib_send_send_video_v2(g_state.NDI_send_ptr, &NDI_video_frame);
		UnloadImage(output_image_ndi);
		
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(GRAY);

		rlImGuiBegin();
		imgui_menu_bar(should_close, show_demo_window);
		imgui_main_app_window();
		if (show_demo_window)
			ImGui::ShowDemoWindow();
		rlImGuiEnd();
		
		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(test_texture);
	UnloadRenderTexture(test_output_target);
	
	rlImGuiShutdown();
	
	if (g_state.NDI_send_ptr)
		NDIlib_send_destroy(g_state.NDI_send_ptr);
	NDIlib_destroy();

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
