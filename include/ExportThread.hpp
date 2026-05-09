#pragma once
#include <atomic>
#include <filesystem>
#include <thread>

struct SequenceInfo;

class ExportThread
{
public:
	enum class State
	{
		NotLaunched,
		Running,
		Failure,
		Success,
	};

	std::atomic<State> state = State::NotLaunched;

	bool running() const noexcept { return state == State::Running; }
	/// Finished, whether successful or not.
	bool finished() const noexcept { return state == State::Failure || state == State::Success; }
	std::optional<std::jthread> launch(std::filesystem::path const& file_path, std::filesystem::path const& textures_path);

private:
	void routine(std::filesystem::path seq_file, std::filesystem::path output_dir);
};
