#include <windows.h>
#include <tchar.h>

#include "ExportThread.hpp"

#include <thread>

extern "C" {
	const char *GetApplicationDirectory(void);
}

namespace fs = std::filesystem;

std::jthread ExportThread::launch(fs::path const& file_path, fs::path const& textures_path)
{
	return std::jthread(&ExportThread::routine, this, file_path, textures_path);
}

// Thanks guys: https://stackoverflow.com/questions/1802471/suppress-console-when-calling-system-in-c
void ExportThread::routine(std::filesystem::path seq_file, std::filesystem::path output_dir)
{
	this->state = State::Running;
	const fs::path exe_path = fs::path(GetApplicationDirectory()) / "ArestiExporter" / "ArestiExporter.exe";
	std::wstring command = std::format(LR"({} --file="{}" --outputdir="{}" --headless)",
		exe_path.wstring(),
		seq_file.wstring(),
		output_dir.wstring()
	);

	STARTUPINFOW startup_info;
	PROCESS_INFORMATION process_info;

	ZeroMemory(&startup_info, sizeof(startup_info));
	startup_info.cb = sizeof(startup_info);
	ZeroMemory(&process_info, sizeof(process_info));

	const BOOL success = CreateProcessW(nullptr, command.data(), nullptr,
		nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr,
		&startup_info, &process_info
	);

	// Process successfully spawned
	if (success)
	{
		// Wait for .exe to be done
		WaitForSingleObject(process_info.hProcess, INFINITE);

		// Get exit code
		DWORD exit_code = 0;
		const BOOL got_exit_code = GetExitCodeProcess(process_info.hProcess, &exit_code);
		CloseHandle(process_info.hProcess);
		CloseHandle(process_info.hThread);

		this->state = (got_exit_code && exit_code == 0) ? State::Success : State::Failure;
	}
	else
	{
		// Failed to spawn process
		this->state = State::Failure;
	}
}
