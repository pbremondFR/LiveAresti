#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include "raylib-cpp.hpp"

extern const char Roboto_Medium_compressed_data_base85[148145 + 1];
extern const char DESG14_Classic_italic_compressed_data_base85[15385+1];

struct ImFont;
struct ImVec2;

void apply_imgui_app_style();
bool NDI_modal();
void side_panel(ImVec2 size);
void NDI_panel(ImVec2 size);
void main_panel();
float imgui_menu_bar(bool &should_close, bool &show_demo_window);
void ndi_export();
void sequence_directory_modal();
void current_sequence_list_modal();

struct SequenceInfo
{
	std::string file_name;
	std::string pilot_name;
	std::string aircraft_type;
	std::string aircraft_reg;
	std::string category;
	std::string program;
	std::string sequence_text; // Probably pretty useless, at least for now
};

struct Figure
{
	raylib::Texture texture_form_b = {};
	raylib::Texture texture_form_c = {};
	int k_factor = 0;
};

struct SequenceData
{
	SequenceInfo info;
	std::vector<Figure> figures;
};

// TODO
enum class SequenceForm
{
	B,
	C,
};

// TODO
enum class SomeFuckingState
{
	Standby,	// Nothing gets shown on the NDI output
	Active,		// Currently showing figures
	Warmup,		// Pilot is warming up before starting the program
	Break,		// Break in a program
};

struct LiveArestiState
{
	std::vector<SequenceData> sequence_list;
	std::filesystem::path sequences_dir;
	size_t current_sequence_idx = 0;

	size_t current_figure_idx = 0;
	size_t desired_figure_index = 0;
	
	std::string NDI_name = "LiveAresti";
	std::string NDI_group = "";

	struct NDIlib_send_instance_type* NDI_send_ptr = nullptr;
	
	bool request_open_ndi_modal = false;
	bool request_open_sequences_modal = false;
	
	// testing/debugging/wip shit
	Texture test_texture;
	RenderTexture2D test_output_target;
};

extern LiveArestiState g_state;
extern ImFont* g_digital_font;
