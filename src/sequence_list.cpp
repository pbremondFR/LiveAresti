#include <filesystem>
#include <imgui.h>
#include <LiveAresti.hpp>
#include <misc/cpp/imgui_stdlib.h>
#include <string>

namespace fs = std::filesystem;

class PathInputText
{
public:
	// Returns true on value changed
	bool operator()(const char* label, const char* hint,
		ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr)
	{
		std::error_code ec;
		const bool exists_before_update = std::filesystem::exists(_path, ec);
		if (!exists_before_update)
			ImGui::PushStyleColor(ImGuiCol_Text, _error_color);

		const bool changed = ImGui::InputTextWithHint(label, hint, &_utf8_string, flags, callback, user_data);

		ImGui::PopStyleColor(exists_before_update ? 0 : 1);
		if (!exists_before_update && ImGui::IsItemHovered())
			ImGui::SetTooltip("Error: %s", ec ? ec.message().c_str() : "Directory does not exist");

		if (changed)
		{
			_path = std::filesystem::u8path(_utf8_string);
		}

		return changed;
	}
	ImVec4 get_error_color() const noexcept { return _error_color; }
	std::filesystem::path const& get_path() const noexcept { return _path; }
	std::string const& get_utf8_string() const noexcept { return _utf8_string; }

	void set_error_color(ImColor color) noexcept { _error_color = color; }
	void set_utf8_string(std::string const& utf8_string) noexcept
	{
		_utf8_string = utf8_string;
		_path = std::filesystem::u8path(_utf8_string);
	}
	void set_path(std::filesystem::path const& path)
	{
		_path = path;
		_utf8_string = reinterpret_cast<const char*>(path.u8string().c_str());
	}

private:
	std::filesystem::path _path;
	std::string _utf8_string; // Cached UTF8 string
	ImVec4 _error_color = {0.9f, 0.0f, 0.0f, 1.0f};
};

static std::vector<SequenceInfo> get_sequences_from_path(fs::path const& path)
{
	std::vector<SequenceInfo> sequences;

	if (!fs::exists(path))
		return sequences;
	for (fs::directory_entry const& entry : fs::directory_iterator(path))
	{
		if (!entry.exists() || !entry.is_regular_file() || entry.path().extension().string() != ".seq")
			continue;
		sequences.emplace_back(SequenceInfo{
			.file_name = reinterpret_cast<const char*>(entry.path().filename().u8string().c_str()),
			.pilot_name = "Jean DUPONT",
			.aircraft_type = "Extra 300SC",
			.aircraft_reg = "F-ABCD",
			.category = "Unlimited",
			.program = "Free Known",
		});
	}
	// NRVO inshallah
	return sequences;
}

void sequence_list_modal()
{
	if (g_state.request_open_sequences_modal)
	{
		ImGui::OpenPopup("Sequence list");
		g_state.request_open_sequences_modal = false;
	}
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Sequence list", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
	{
		static bool path_exists = false;
		static PathInputText sequences_path_widget;
		static std::vector<SequenceInfo> sequences_in_dir;

		// Don't want the length of the path selector to be too short
		ImGui::SetNextItemWidth(500);
		const bool changed = sequences_path_widget("Sequences path", "e.g. C:\\Documents\\OpenAeroSequences",
			ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_ElideLeft);
		if (changed)
		{
			path_exists = fs::exists(sequences_path_widget.get_path());
			sequences_in_dir = get_sequences_from_path(sequences_path_widget.get_path());
		}

		ImGui::Separator();

		static constexpr ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("Sequence list", 7, table_flags, {0, 400}))
		{
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
				SequenceInfo const& seq = sequences_in_dir[i];

				ImGui::TableNextRow();
				ImGui::PushID(seq.file_name.c_str());
				ImGui::TableSetColumnIndex(0); ImGui::Text("%d", i);
				ImGui::TableSetColumnIndex(1); ImGui::TextUnformatted(seq.file_name.c_str());
				ImGui::TableSetColumnIndex(2); ImGui::TextUnformatted(seq.pilot_name.c_str());
				ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(seq.program.c_str());
				ImGui::TableSetColumnIndex(4); ImGui::TextUnformatted(seq.category.c_str());
				ImGui::TableSetColumnIndex(5); ImGui::TextUnformatted(seq.aircraft_type.c_str());
				ImGui::TableSetColumnIndex(6); ImGui::Selectable(seq.aircraft_reg.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_DontClosePopups);

				ImGui::PopID();
				if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
				{
					int n_next = i + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
					if (n_next >= 0 && n_next < sequences_in_dir.size())
					{
						std::swap(sequences_in_dir[i], sequences_in_dir[n_next]);
						ImGui::ResetMouseDragDelta();
					}
				}
			}
            ImGui::PopItemFlag();
			ImGui::EndTable();
		}

		ImGui::Separator();

		if (ImGui::Button("Save"))
		{
			g_state.sequence_list = sequences_in_dir;
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
