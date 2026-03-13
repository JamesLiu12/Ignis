#include "ProcessRunner.h"
#include "Ignis/Core/Log.h"
#include <array>

#ifdef _WIN32
#define popen  _popen
#define pclose _pclose
#endif

namespace ignis 
{

	ProcessRunner::RunResult ProcessRunner::RunCommand(
		const std::string& command,
		const std::filesystem::path& working_directory)
	{
		RunResult result;
		auto original_path = std::filesystem::current_path();

		if (!working_directory.empty())
		{
			try
			{
				std::filesystem::current_path(working_directory);
			}
			catch (const std::exception&)
			{
				Log::CoreError("Failed to change directory to: {}", working_directory.string());
				return result;
			}
		}

		std::array<char, 256> buffer;

		FILE* pipe = popen(command.c_str(), "r");
		if (!pipe)
		{
			std::filesystem::current_path(original_path);
			Log::CoreError("Failed to execute command: {}", command);
			return result;
		}

		while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
			result.Output += buffer.data();

		result.ExitCode = pclose(pipe);
		result.Success = (result.ExitCode == 0);

		std::filesystem::current_path(original_path);
		return result;
	}

} // namespace ignis