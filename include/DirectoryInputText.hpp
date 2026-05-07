#pragma once

#include <filesystem>
#include <imgui.h>
#include <system_error>
#include <misc/cpp/imgui_stdlib.h>

namespace widget
{
/**
 * An ImGui widget that lets the user enter a directory path. When directory does not exist, widget text turns red
 * and a tooltip displays an error.
 *
 * TODO: Refactor this into something more generic, maybe? Take a std::filesystem::file_type to handle stuff
 * other than directories, idk
 */
class DirectoryInputText
{
public:
	// Returns true on value changed
	bool operator()(const char* label, const char* hint,
		ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr)
	{
		std::error_code ec;
		const bool exists_before_update = std::filesystem::exists(_path, ec);
		const bool is_directory = std::filesystem::is_directory(_path, ec);
		if (!exists_before_update || !is_directory)
			ImGui::PushStyleColor(ImGuiCol_Text, _error_color);

		const bool changed = ImGui::InputTextWithHint(label, hint, &_utf8_string, flags, callback, user_data);

		ImGui::PopStyleColor(!exists_before_update || !is_directory ? 1 : 0);
		if (ImGui::IsItemHovered())
		{
			if (!exists_before_update)
				ImGui::SetTooltip("Error: %s", ec ? ec.message().c_str() : "Directory does not exist");
			else if (!is_directory)
				ImGui::SetTooltip("Error: %s", ec ? ec.message().c_str() : "Is not a directory");
		}

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

}
