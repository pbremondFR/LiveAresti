#pragma once
#include <string>
#include <vector>
#include "raylib.h"

extern const char Roboto_Medium_compressed_data_base85[148145 + 1];

void apply_imgui_app_style();
bool NDI_modal();
void side_panel(struct ImVec2 size);
void NDI_panel(struct ImVec2 size);
void main_panel();
float imgui_menu_bar(bool &should_close, bool &show_demo_window);
void ndi_export();

struct SequenceInfo
{
	std::string pilot_name;
	std::string aircraft_type;
	std::string aircraft_reg;
	std::string category;
	std::string program;
};

struct LiveArestiState
{
	SequenceInfo sequence_info;
	std::vector<int> figures = {};
	size_t current_figure_idx = 0;
	size_t desired_figure_index = 0;
	
	std::string NDI_name = "LiveAresti";
	std::string NDI_group = "";

	struct NDIlib_send_instance_type* NDI_send_ptr = nullptr;
	
	bool request_open_ndi_modal = false;
	
	// testing/debugging/wip shit
	Texture test_texture;
	RenderTexture2D test_output_target;
};

extern LiveArestiState g_state;
