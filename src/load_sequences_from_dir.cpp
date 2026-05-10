#include <filesystem>
#include <fstream>
#include <functional>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <LiveAresti.hpp>
#include <ranges>
#include <string>
#include <thread>

#include "DirectoryInputText.hpp"
#include "ExportThread.hpp"
#include "pugixml.hpp"

namespace fs = std::filesystem;

/// File names of images to load from disk to GPU once the list is saved.
struct FigureFilenames
{
	std::string filename_form_b;
	std::string filename_form_c;
	int k_factor = 0;

	/// Loads textures to GPU VRAM from file names. Will block the current thread, as such isn't very efficient.
	[[nodiscard]] std::optional<Figure> load_textures_in_vram() const noexcept
	{
		// printf("%s: %s\n", __FUNCTION__, filename_form_b.c_str());
		Figure new_figure = {
			.texture_form_b = LoadTexture(filename_form_b.c_str()),
			.texture_form_c = LoadTexture(filename_form_c.c_str()),
			.k_factor = k_factor,
		};
		if (!new_figure.texture_form_b.IsValid() || !new_figure.texture_form_c.IsValid())
			return std::nullopt;
		return new_figure;
	}
};

/// Intermediate representation of a figure by holding the form B and C *Images* (that is, image files loaded into
/// RAM). This will end up being converted into Textures (use load_textures_in_vram())
struct FigureImages
{
	raylib::Image image_form_b;
	raylib::Image image_form_c;
	int k_factor = 0;

	[[nodiscard]] std::optional<Figure> load_textures_in_vram() const noexcept
	{
		Figure figure = {
			.texture_form_b = LoadTextureFromImage(image_form_b),
			.texture_form_c = LoadTextureFromImage(image_form_c),
			.k_factor = k_factor,
		};
		if (!figure.texture_form_b.IsValid() || !figure.texture_form_c.IsValid())
			return std::nullopt;
		else
			return figure;
	}
};

/// Temporary data about the sequence, which will be converted into full SequenceData when needed.
/// Instead of storing a list of figures, it just stores their texture filenames, to be loaded in VRAM later.
struct SequenceTemporaryData
{
	SequenceInfo info = {};
	std::vector<FigureFilenames> figure_filenames = {};
	size_t hash = 0;

	/// Whether files from this sequence are already cached (found in the directory of get_exported_textures_path)
	[[nodiscard]] bool are_files_cached() const noexcept { return !figure_filenames.empty(); }

	/// Loads all the figure images into RAM. Returns the corresponding vector, or an empty optional if one or more
	/// images failed to load.
	[[nodiscard]] auto load_sequence_images_to_RAM() const -> std::optional<std::vector<FigureImages>>
	{
		std::vector<FigureImages> result;

		for (FigureFilenames const& filenames : figure_filenames)
		{
			FigureImages images = {
				.image_form_b = LoadImage(filenames.filename_form_b.c_str()),
				.image_form_c = LoadImage(filenames.filename_form_c.c_str()),
				.k_factor = filenames.k_factor,
			};

			if (images.image_form_b.data && images.image_form_c.data)
				result.emplace_back(images);
			else
				return std::nullopt;
		}
		return result;
	}
};

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

/**
 * Returns the directory where sequence images are located based on .seq file hash
 *
 * @param file_hash Hash of .seq file
 * @return Filesystem path of directory where sequence images should be exported & fetched
 */
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
static std::vector<FigureFilenames> fetch_exported_textures_of_sequence(fs::path const& textures_path, pugi::xml_node const& seq)
{
	std::vector<FigureFilenames> sequences;

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
		FigureFilenames figure = {
			.filename_form_b = form_b_filenames[i].c_str(),
			.filename_form_c = form_c_filenames[i].c_str(),
			.k_factor = k_factors[i],
		};
		// One texture couldn't be loaded into memory, don't want a corrupted image list so discard this program entirely...
		if (figure.filename_form_b.empty() || figure.filename_form_c.empty())
			return {};
		sequences.push_back(std::move(figure));
	}
	return sequences;
}

/**
 * Looks into the given directory and returns a list of all found OpenAero sequences.
 * @param directory Folder in which to scan for .seq files
 * @return vector of sequence temporary data (contains texture filenames instead of loaded textures).
 */
static std::vector<SequenceTemporaryData> get_sequences_from_path(fs::path const& directory)
{
	std::vector<SequenceTemporaryData> sequences;

	if (!fs::is_directory(directory))
		return sequences;
	for (fs::directory_entry const& entry : fs::directory_iterator(directory))
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
		std::vector<FigureFilenames> figures = fetch_exported_textures_of_sequence(texture_dir, seq);
		sequences.emplace_back(SequenceTemporaryData{
			.info = SequenceInfo{
				.file_name = reinterpret_cast<const char*>(entry.path().stem().u8string().c_str()),
				.pilot_name = seq.child("pilot").text().as_string("???"),
				.aircraft_type = seq.child("actype").text().as_string("???"),
				.aircraft_reg = seq.child("acreg").text().as_string("???"),
				.category = seq.child("category").text().as_string("???"),
				.program = seq.child("program").text().as_string("???"),
				.sequence_text = seq.child("sequence_text").text().as_string(""),
				.number_of_figures = static_cast<int>(figures.size()),
			},
			.figure_filenames = std::move(figures),
			.hash = *file_hash,
		});
	}
	// NRVO inshallah
	return sequences;
}

/// ImGui SmallButton that reflects a sequence's load status (as reflected by ExportThread::State)
static bool sequence_load_button(bool has_cached_textures, ExportThread::State thread_state)
{
	static constexpr ImU32 good = IM_COL32(0, 200, 0, 255);
	static constexpr ImU32 bad = IM_COL32(200, 0, 0, 255);

	const char* button_text = "";
	if (has_cached_textures)
		thread_state = ExportThread::State::Success;
	switch (thread_state)
	{
		case ExportThread::State::Failure:
			button_text = "Failed";	break;
		case ExportThread::State::Running:
			button_text = "...";	break;
		case ExportThread::State::NotLaunched:
			button_text = "Load";	break;
		case ExportThread::State::Success:
			button_text = "Loaded";	break;
	}

	ImGui::BeginDisabled(has_cached_textures || thread_state == ExportThread::State::Running);
	if (has_cached_textures)
		ImGui::PushStyleColor(ImGuiCol_Button, good);
	else if (thread_state == ExportThread::State::Failure)
		ImGui::PushStyleColor(ImGuiCol_Button, bad);

	ImGui::PushStyleVarY(ImGuiStyleVar_FramePadding, 0.0f);
	// ImGui::SmallButton, but with custom size
	const bool clicked = ImGui::ButtonEx(button_text, {60, 0}, ImGuiButtonFlags_AlignTextBaseLine);
	ImGui::PopStyleVar();

	if (has_cached_textures || thread_state == ExportThread::State::Failure)
		ImGui::PopStyleColor();
	ImGui::EndDisabled();

	return clicked;
}

/// Erases successful finished threads from g_state.export_threads. Returns true if one or more thread was erased.
static bool erase_successful_finished_threads()
{
	const size_t num_threads_finished = std::erase_if(g_state.export_threads,
	[](std::pair<const size_t, ExportThread> const& item)
	{
		auto const& thread = item.second;
		return thread.state.load() == ExportThread::State::Success;
	});
	return num_threads_finished > 0;
}

static void do_export_all_dotseq_files(
	fs::path const& sequences_dir,
	std::vector<SequenceTemporaryData> const& sequences_in_dir
	)
{
	for (SequenceTemporaryData const& sequence : sequences_in_dir)
	{
		if (sequence.are_files_cached())
			continue;
		const fs::path file_path = sequences_dir / (sequence.info.file_name + ".seq");
		const fs::path textures_path = get_exported_textures_path(sequence.hash);
		if (auto thread = g_state.export_threads[sequence.hash].launch(file_path, textures_path))
		{
			thread->detach();
		}
	}
}

/**
 * For all sequences in directory, loads all their image files into RAM. Stores the result into a map, associating
 * sequence hash to images. The reason this doesn't straight up go to VRAM is because we need to put this in a thread,
 * and we can't load into VRAM from somewhere else than the main thread.
 *
 * @param sequences_in_dir All sequences in the directory
 * @param loaded_images Pointer to map associating sequence hash to sequence images
 * @param loaded_progress Float representing load progress for 0 to 1.
 */
static void load_all_sequences_images_to_ram(
	std::vector<SequenceTemporaryData> sequences_in_dir,
	std::atomic<std::shared_ptr<std::unordered_map<size_t, std::vector<FigureImages>>>>& loaded_images,
	std::atomic<float>& loaded_progress
	)
{
	// Don't run thread if one is already running
	if (loaded_images.load() != nullptr)
		return;

	// Working copy where we can wait all we want for image loading
	auto working_map = std::make_shared<std::unordered_map<size_t, std::vector<FigureImages>>>();

	loaded_progress = 0.0f;
	const float delta = 1.0f / static_cast<float>(sequences_in_dir.size());
	for (SequenceTemporaryData const& temp_data : sequences_in_dir)
	{
		if (auto images_in_ram = temp_data.load_sequence_images_to_RAM())
			working_map->emplace(temp_data.hash, std::move(*images_in_ram));
		loaded_progress += delta;
	}
	// Exchange pointers
	loaded_images = working_map;
}

/**
 * From current sequences that are in directory and previously loaded sequences images, creates the final
 * vector of SequenceData by moving images into VRAM.
 *
 * @param sequences_in_dir All sequences in directory
 * @param sequence_images Map associating sequence hash to previously loaded sequence images (loaded into RAM)
 * @return Final vector of sequences to be used by the application
 */
static std::vector<SequenceData> load_all_sequence_textures_to_vram(
	std::vector<SequenceTemporaryData> sequences_in_dir,
	std::unordered_map<size_t, std::vector<FigureImages>> const& sequence_images
	)
{
	// Transforms a vector of images to a vector of textures. Returns nullopt if one or more load fails.
	auto load_textures_to_vram = [](std::vector<FigureImages> const& images) -> std::optional<std::vector<Figure>>
	{
		std::vector<Figure> result;
		for (FigureImages const& image : images)
		{
			if (auto textures = image.load_textures_in_vram(); textures.has_value())
				result.emplace_back(std::move(*textures));
			else
				return std::nullopt;
		}
		return result;
	};
	std::vector<SequenceData> new_sequences;

	for (SequenceTemporaryData const& temp_data : sequences_in_dir)
	{
		auto images = sequence_images.find(temp_data.hash);
		if (images == sequence_images.end())
			continue; // Image never loaded in the RAM, skip the sequence

		auto textures = load_textures_to_vram(images->second);
		if (!textures.has_value())
			continue; // Failed to move images to GPU VRAM

		new_sequences.emplace_back(SequenceData{
			.info = temp_data.info,
			.figures = std::move(*textures),
			.hash = temp_data.hash,
		});
	}
	return new_sequences;
}

// TODO: Add a progress bar for loading (long time if script needs to be ran!!!). Other thread?
// TODO: Differential move based on sequence hash (+ file name?) to only save sequences that are newly added
//  (avoids long freeze every time). Not very important as we're unlikely to do anything else than one big load-all.
// TODO: Progress bar for save? Means saving on other thread & swapping vectors once it's all loaded
void sequence_directory_modal()
{
	// One exporter thread ended, time to refresh the file list
	bool rescan_files = false;
	// Do this shit instead of normal OpenPopup usage because the button is located inside a menu of the main menu bar,
	// where the code below would never get drawn, because the menu would close on the frame when the button is pressed.
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
		static std::vector<SequenceTemporaryData> sequences_in_dir;
		static std::atomic<std::shared_ptr<std::unordered_map<size_t, std::vector<FigureImages>>>> loaded_sequences = nullptr;
		static std::atomic<float> loaded_progress = 0.0f;

		if (erase_successful_finished_threads())
			rescan_files = true;

		if (ImGui::Button("Reload"))
			rescan_files = true;
		ImGui::SameLine();
		// Don't want the length of the path selector to be too short
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
		rescan_files = rescan_files || sequences_path_widget("##sequences_path", "Sequences path",
			ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_ElideLeft);
		if (rescan_files)
		{
			sequences_in_dir = get_sequences_from_path(sequences_path_widget.get_path());
		}

		ImGui::Separator();

		static constexpr ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("Sequence list", 8, table_flags, {0, 400}))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Top row is sticky (always visible)
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
				SequenceTemporaryData const& sequence_data = sequences_in_dir[i];
				SequenceInfo const& sequence_info = sequences_in_dir[i].info;

				// Check if a thread is associated with this sequence
				auto thread_it = g_state.export_threads.find(sequence_data.hash);
				ExportThread::State thread_state = thread_it != g_state.export_threads.end()
					? thread_it->second.state.load()
					: ExportThread::State::NotLaunched;

				ImGui::TableNextRow();
				ImGui::PushID(sequence_info.file_name.c_str());
				ImGui::TableSetColumnIndex(0);
				if (sequence_load_button(sequence_data.are_files_cached(), thread_state))
				{
					// Launch thread to export images with ArestiExporter
					const fs::path file_path = sequences_path_widget.get_path() / (sequence_info.file_name + ".seq");
					const fs::path textures_path = get_exported_textures_path(sequence_data.hash);
					// Deref optional here, assume thread is never in map because button made it available
					g_state.export_threads[sequence_data.hash].launch(file_path, textures_path)->detach();
				}
				// Disable text for rest of the row so it's grey, but we can still click the button
				ImGui::BeginDisabled(!sequence_data.are_files_cached());
				ImGui::TableSetColumnIndex(1); ImGui::Text("%d", i);
				ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(sequence_info.file_name.c_str());
				ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(sequence_info.pilot_name.c_str());
				ImGui::TableSetColumnIndex(4); ImGui::TextUnformatted(sequence_info.program.c_str());
				ImGui::TableSetColumnIndex(5); ImGui::TextUnformatted(sequence_info.category.c_str());
				ImGui::TableSetColumnIndex(6); ImGui::TextUnformatted(sequence_info.aircraft_type.c_str());
				ImGui::TableSetColumnIndex(7); ImGui::TextUnformatted(sequence_info.aircraft_reg.c_str());
				ImGui::EndDisabled();
				ImGui::PopID();
			}
            ImGui::PopItemFlag();
			ImGui::EndTable();
		}

		ImGui::Separator();

		if (ImGui::Button("Save"))
		{
			// Loading images to RAM is very slow, spin up a thread to do it in parallel. It would be nice to load
			// into VRAM in the thread too, but that's not possible due to Raylib + OpenGL limitations:
			// https://www.reddit.com/r/raylib/comments/lxihj5/loadtexture_doesnt_work_in_a_separate_thread_c/
			std::thread(&load_all_sequences_images_to_ram,
				sequences_in_dir, std::ref(loaded_sequences), std::ref(loaded_progress)
				).detach();
			// Open popup which will stay on until loading is done
			ImGui::OpenPopup("Loading sequences...");
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			sequences_path_widget.set_path(g_state.sequences_dir);
			sequences_in_dir = get_sequences_from_path(g_state.sequences_dir);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine(); ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical); ImGui::SameLine();
		if (ImGui::Button("Load all sequences"))
		{
			do_export_all_dotseq_files(sequences_path_widget.get_path(), sequences_in_dir);
		}
		// Progress bar modal, blocking all user interactions until all images have been loaded into the program
		if (ImGui::BeginPopupModal("Loading sequences...", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::ProgressBar(loaded_progress.load(), {500, 0});
			// Finished loading all sequences images to RAM (that's slow), now just move stuff to VRAM (that's fast)
			if (loaded_sequences.load() != nullptr)
			{
				g_state.sequence_list = load_all_sequence_textures_to_vram(sequences_in_dir, *loaded_sequences.load());
				g_state.sequences_dir = sequences_path_widget.get_path();
				loaded_sequences = nullptr;
				// Only close (both) popups & give control back when we've finished loading everything
				ImGui::ClosePopupToLevel(0, true);
			}
			ImGui::EndPopup();
		}
		ImGui::EndPopup();
	}
}
