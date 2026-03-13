#pragma once
#include <filesystem>
#include <string>

namespace ignis 
{

	class ProcessRunner
	{
	public:
		struct RunResult
		{
			bool Success = false;
			int ExitCode = 0;
			std::string Output;
		};

		static RunResult RunCommand(
			const std::string& command,
			const std::filesystem::path& working_directory = {});
	};

} // namespace ignis