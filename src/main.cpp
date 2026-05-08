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

LiveArestiState g_state = {};
ImFont* g_digital_font = nullptr;

static float menu_bar_height = 0.0f;
static bool should_close = false;
static bool show_demo_window = false;

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

	ImGui::Begin("main", nullptr, flags);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
	menu_bar_height = imgui_menu_bar(should_close, show_demo_window);
	// Modals that open within the menu bar.
	sequence_directory_modal();
	NDI_modal();

	const ImVec2 origin = {ImGui::GetStyle().WindowPadding.x, menu_bar_height + ImGui::GetStyle().WindowPadding.y};
	const float side_panel_width = viewport->WorkSize.x - 600 - ImGui::GetStyle().WindowPadding.x*2 - ImGui::GetStyle().ItemSpacing.y; 
	const ImVec2 NDI_panel_size = {side_panel_width, 70};
	const ImVec2 side_panel_size = {side_panel_width, 600 - NDI_panel_size.y - ImGui::GetStyle().ItemSpacing.y};
	{
		ImGui::SetNextWindowPos(origin);
		NDI_panel(NDI_panel_size);
	}
	{
		ImGui::SetNextWindowPos({
			ImGui::GetStyle().WindowPadding.x,
			origin.y + NDI_panel_size.y + ImGui::GetStyle().ItemSpacing.y
		});
		side_panel(side_panel_size);
	}
	{
		ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(127, 127, 127, 255));
		ImGui::SetNextWindowPos({viewport->WorkSize.x - 600 - ImGui::GetStyle().WindowPadding.x, origin.y});
		ImGui::BeginChild("preview", {600, 600});
		rlImGuiImageRenderTexture(&g_state.test_output_target);
		ImGui::EndChild();
		ImGui::PopStyleColor();
	}
	{
		main_panel();
	}

	ImGui::PopStyleVar();
	ImGui::End();
}

int main()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);
	SetTraceLogLevel(LOG_WARNING);

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
	g_state.test_texture = LoadTexture("MIKA BRAGEOT_Free Known_FormB_Fig01.png");
	g_state.test_output_target = LoadRenderTexture(600, 600);
	
	const Shader border_mask_shader = LoadShader(nullptr, "border_mask.frag");
	
	// game loop
	// run the loop until the user presses ESCAPE or presses the Close button on the window
	while (!WindowShouldClose() && !should_close)
	{
		BeginTextureMode(g_state.test_output_target);
		BeginShaderMode(border_mask_shader);		
		ClearBackground(BLANK);
		DrawTexturePro(g_state.test_texture,
			{ 0.0f, 0.0f, (float)g_state.test_texture.width, (float)g_state.test_texture.height },
			{ 600.0f, 600.0f, (float)g_state.test_texture.width, (float)g_state.test_texture.height },
			{600, 600},
			// static_cast<float>(GetTime() * 100),
			0,
			WHITE);
		EndShaderMode();
		EndTextureMode();
		
		ndi_export();
		
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(GRAY);

		rlImGuiBegin();
		imgui_main_app_window();
		if (show_demo_window)
			ImGui::ShowDemoWindow();
		rlImGuiEnd();
		
		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(g_state.test_texture);
	UnloadRenderTexture(g_state.test_output_target);
	
	rlImGuiShutdown();
	
	if (g_state.NDI_send_ptr)
		NDIlib_send_destroy(g_state.NDI_send_ptr);
	NDIlib_destroy();

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
