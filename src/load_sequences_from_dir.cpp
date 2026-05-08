#include <filesystem>
#include <fstream>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <LiveAresti.hpp>
#include <ranges>
#include <string>

#include "DirectoryInputText.hpp"
#include "pugixml.hpp"

namespace fs = std::filesystem;

static std::optional<size_t> get_file_hash(fs::path const& dotseq_file_path)
{
	std::ifstream filestream(dotseq_file_path.string());
	if (!filestream)
		return std::nullopt;
	std::stringstream textstream;
	textstream << filestream.rdbuf();
	size_t hash = std::hash<std::string>{}(textstream.str());
	return hash;
}

/**
 * Looks at the contents of a .seq file and computes a directory matching the unique contents of the file.
 * Internally, hashes the file contents and uses that hash for the directory name.
 *
 * @param dotseq_file_path .seq file path
 * @return Filesystem path of directory where sequence images should be exported & fetched.
 */
static std::optional<fs::path> get_exported_textures_path(fs::path const& dotseq_file_path)
{
	std::optional<size_t> hash = get_file_hash(dotseq_file_path);
	if (!hash.has_value())
		return std::nullopt;
	fs::path texture_dir = fs::current_path().parent_path() / "exported_images" / std::to_string(*hash);
	return texture_dir;
}

static fs::path get_exported_textures_path(size_t file_hash)
{
	return fs::current_path().parent_path() / "exported_images" / std::to_string(file_hash);
}

/**
 * Look into given directory for exported image files of a given sequence, for both forms B and C. If they're
 * found, load them into VRAM and return a vector of Figure objects.
 *
 * @param textures_path Filesystem path where textures are expected to be present.
 * @param seq XML <sequence> node object, so we can lookup properties such as K-factors for each figure
 * @return Vector of figures, all loaded into the GPU. Empty if not found or unexpected error.
 */
static std::vector<Figure> fetch_exported_textures_of_sequence(fs::path const& textures_path, pugi::xml_node const& seq)
{
	std::vector<Figure> sequences;

	const fs::path form_b_path = textures_path / "Form_B";
	const fs::path form_c_path = textures_path / "Form_C";
	if (!fs::is_directory(form_b_path) || !fs::is_directory(form_c_path))
		return {};

	// Get list of all filenames
	std::vector<std::string> form_b_filenames;
	std::vector<std::string> form_c_filenames;
	for (fs::directory_entry const& entry : fs::directory_iterator(form_b_path))
		form_b_filenames.push_back(entry.path().string());
	for (fs::directory_entry const& entry : fs::directory_iterator(form_c_path))
		form_c_filenames.push_back(entry.path().string());
	// Should have the strict same amount of files in both forms
	if (form_b_filenames.size() != form_c_filenames.size())
		return {};

	// Sort to ensure correct order of textures (files are named in order)
	std::sort(form_b_filenames.begin(), form_b_filenames.end());
	std::sort(form_c_filenames.begin(), form_c_filenames.end());

	// Store K-factor of each figure
	std::vector<int> k_factors;
	for (auto const& figure : seq.child("figures").children("figure"))
		k_factors.push_back(figure.child("figk").text().as_int(0));
	// If amount of figures in .seq differs from amount of images, there's a mismatch between .seq & directory, or
	// some images are missing from both forms B and C
	if (k_factors.size() != form_b_filenames.size())
		return {};

	// Load textures for each texture of forms B and C
	for (int i = 0; i < form_b_filenames.size(); i++)
	{
		Figure figure = {
			.texture_form_b = LoadTexture(form_b_filenames[i].c_str()),
			.texture_form_c = LoadTexture(form_c_filenames[i].c_str()),
			.k_factor = k_factors[i],
		};
		// One texture couldn't be loaded into memory, don't want a corrupted image list so discard this program entirely...
		if (figure.texture_form_b.id == 0 || figure.texture_form_c.id == 0)
			return {};
		sequences.push_back(std::move(figure));
	}
	return sequences;
}

static std::vector<SequenceData> get_sequences_from_path(fs::path const& path)
{
	std::vector<SequenceData> sequences;

	if (!fs::is_directory(path))
		return sequences;
	for (fs::directory_entry const& entry : fs::directory_iterator(path))
	{
		if (!entry.exists() || !entry.is_regular_file() || entry.path().extension().string() != ".seq")
			continue;

		std::optional<size_t> file_hash = get_file_hash(entry.path());
		if (!file_hash.has_value())
			continue;
		fs::path texture_dir = fs::current_path().parent_path() / "exported_images" / std::to_string(*file_hash);

		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(entry.path().string().c_str());
		if (!result || doc.child("sequence").empty())
			continue;
		pugi::xml_node seq = doc.child("sequence");
		sequences.emplace_back(SequenceData{
			.info = SequenceInfo{
				.file_name = reinterpret_cast<const char*>(entry.path().stem().u8string().c_str()),
				.pilot_name = seq.child("pilot").text().as_string("???"),
				.aircraft_type = seq.child("actype").text().as_string("???"),
				.aircraft_reg = seq.child("acreg").text().as_string("???"),
				.category = seq.child("category").text().as_string("???"),
				.program = seq.child("program").text().as_string("???"),
				.sequence_text = seq.child("sequence_text").text().as_string(""),
			},
			.figures = fetch_exported_textures_of_sequence(texture_dir, seq),
			.hash = *file_hash,
		});
	}
	// NRVO inshallah
	return sequences;
}

static bool sequence_load_button(bool loaded)
{
	if (loaded)
	{
		ImGui::BeginDisabled();
		ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 200, 0, 255));
	}
	bool clicked = ImGui::SmallButton("Load");
	if (loaded)
	{
		ImGui::PopStyleColor();
		ImGui::EndDisabled();
	}
	return clicked;
}

// Pretty shit way of doing things rn
static int exporter_thread_routine(fs::path seq_file, fs::path const& output_dir, std::atomic_bool& thread_ended)
{
	std::string command = std::format("node \"{}\" --file=\"{}\" --outputdir=\"{}\"",
		"E:\\Documents\\Code\\ArestiExporter\\export_program.js",
		seq_file.string(),
		output_dir.string()
	);
	puts(command.c_str());
	int retcode = std::system(command.c_str());
	return retcode;
}

// TODO: Change the design. There should be a manual reload from the path.
// Add a button to load images for all programs
// Add a progress bar for loading (long time if script needs to be ran!!!). Other thread?
// Load images to GPU only after hitting save? Avoids lag when opening/closing window.
void sequence_directory_modal()
{
	// One exporter thread ended, time to refresh the file list
	std::atomic_bool exporter_thread_ended = false;
	bool rescan_files = false;
	if (g_state.request_open_sequences_modal)
	{
		ImGui::OpenPopup("Sequence list");
		g_state.request_open_sequences_modal = false;
		rescan_files = true;
	}
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	// Do a manual check for the open popup here instead of using ImGuiCond_Appearing to allow for the dynamic resizing
	// of the window. Otherwise position is never updated when the window size changes (due to the table filling).
	if (ImGui::IsPopupOpen("Sequence list"))
		ImGui::SetNextWindowPos(center, 0, {0.5, 0.5});

	if (ImGui::BeginPopupModal("Sequence list", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		static widget::DirectoryInputText sequences_path_widget;
		static std::vector<SequenceData> sequences_in_dir;

		// Don't want the length of the path selector to be too short
		ImGui::SetNextItemWidth(500);
		rescan_files = rescan_files || sequences_path_widget("Sequences path", "e.g. C:\\Documents\\OpenAeroSequences",
			ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_ElideLeft);
		if (rescan_files)
		{
			sequences_in_dir = get_sequences_from_path(sequences_path_widget.get_path());
		}

		ImGui::Separator();

		static constexpr ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("Sequence list", 8, table_flags, {0, 400}))
		{
			ImGui::TableSetupColumn(nullptr, ImGuiTableColumnFlags_WidthFixed);
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
				SequenceData const& sequence_data = sequences_in_dir[i];
				SequenceInfo const& sequence_info = sequences_in_dir[i].info;

				ImGui::TableNextRow();
				ImGui::PushID(sequence_info.file_name.c_str());
				ImGui::TableSetColumnIndex(0); bool do_load = sequence_load_button(sequences_in_dir[i].figures.size() > 0);
				if (do_load)
				{
					fs::path file_path = sequences_path_widget.get_path() / (sequence_info.file_name + ".seq");
					auto thread = std::jthread(&exporter_thread_routine,
						file_path, get_exported_textures_path(sequence_data.hash), std::ref(exporter_thread_ended));
					thread.detach();
				}
				ImGui::TableSetColumnIndex(1); ImGui::Text("%d", i);
				ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(sequence_info.file_name.c_str());
				ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(sequence_info.pilot_name.c_str());
				ImGui::TableSetColumnIndex(4); ImGui::TextUnformatted(sequence_info.program.c_str());
				ImGui::TableSetColumnIndex(5); ImGui::TextUnformatted(sequence_info.category.c_str());
				ImGui::TableSetColumnIndex(6); ImGui::TextUnformatted(sequence_info.aircraft_type.c_str());
				ImGui::TableSetColumnIndex(7); ImGui::TextUnformatted(sequence_info.aircraft_reg.c_str());
				ImGui::PopID();
			}
            ImGui::PopItemFlag();
			ImGui::EndTable();
		}

		ImGui::Separator();

		if (ImGui::Button("Save"))
		{
			std::erase_if(sequences_in_dir, [](SequenceData const& seq) { return seq.figures.empty(); });
			g_state.sequence_list = std::move(sequences_in_dir);
			// Reset vector after std::move, just to be sure it's valid if it doesn't get re-computed (it should,
			// because next time popup is open it will rescan files, but better safe than sorry)
			sequences_in_dir.clear();
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
