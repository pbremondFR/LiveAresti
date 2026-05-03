/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"
#include "imgui_style.hpp"
#include "LiveAresti.hpp"
#include "path_utils/resource_dir.h"	// utility header for SearchAndSetResourceDir

Texture test_texture;
RenderTexture2D test_output_target;

void imgui_menu_bar(bool &should_close, bool &show_demo_window)
{
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Quit"))
				should_close = true;
 
			ImGui::EndMenu();
		}
 
		if (ImGui::BeginMenu("Window")) {
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
	
	ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(127, 127, 127, 255));
	ImGui::SetNextWindowPos({(ImGui::GetContentRegionAvail().x - 600.0f) / 2, 30});
	ImGui::BeginChild("preview", {600, 600});
	rlImGuiImageRenderTexture(&test_output_target);
	ImGui::EndChild();
	ImGui::PopStyleColor();
	

	ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0, 64, 64, 255));
	ImGui::BeginChild("main_controls");
	ImGui::Text("Hello, world!");
	ImGui::Text("This is where the controls should go.");
	ImGui::Button("← FORM B");
	ImGui::Button("FORM C →");
	ImGui::EndChild();
	ImGui::PopStyleColor();

	ImGui::End();
}

int main()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);

	// Create the window and OpenGL context
	InitWindow(1000, 1000, "Hello Raylib");
	
	rlImGuiSetup(true);
	apply_app_style();

	auto& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.Fonts->Clear();
	io.Fonts->AddFontFromMemoryCompressedBase85TTF(Roboto_Medium_compressed_data_base85, 17);
	io.Fonts->Build();

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// Load a texture from the resources directory
	test_texture = LoadTexture("MIKA BRAGEOT_Free Known_FormB_Fig01.png");
	
	test_output_target = LoadRenderTexture(600, 600);
	
	bool should_close = false;
	bool show_demo_window = false;
	
	// game loop
	while (!WindowShouldClose() && !should_close)		// run the loop until the user presses ESCAPE or presses the Close button on the window
	{
		BeginTextureMode(test_output_target);
		ClearBackground(BLANK); // Transparence totale
		// DrawTextureEx(test_texture, {0, 0}, static_cast<float>(GetTime() * 10), 1.0f, WHITE);
		DrawTexturePro(test_texture,
			{ 0.0f, 0.0f, (float)test_texture.width, (float)test_texture.height },
			{ 300.0f, 300.0f, (float)test_texture.width, (float)test_texture.height },
			{300, 300},
			static_cast<float>(GetTime() * 30),
			WHITE);
		EndTextureMode();
		
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(GRAY);

		// draw some text using the default font
		DrawText("Hello Raylib from C++",  200, 200, 20, WHITE);

		// draw our texture to the screen
		// DrawTexture(wabbit, 200, 300, WHITE);
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

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
