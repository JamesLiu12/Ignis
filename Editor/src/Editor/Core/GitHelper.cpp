#include "GitHelper.h"
#include "Ignis/Core/Log.h"

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

namespace ignis {

bool GitHelper::IsGitAvailable()
{
	std::string output;
	bool result = RunGitCommand("git --version", std::filesystem::current_path(), &output);
	if (result)
	{
		Log::CoreInfo("Git is available: {}", output);
	}
	return result;
}

bool GitHelper::RunGitCommand(
	const std::string& command,
	const std::filesystem::path& working_directory,
	std::string* output)
{
	// Change to working directory and run command
	auto original_path = std::filesystem::current_path();
	
	try
	{
		std::filesystem::current_path(working_directory);
	}
	catch (const std::exception& e)
	{
		Log::CoreError("Failed to change directory to: {}", working_directory.string());
		return false;
	}
	
	// Execute command and capture output
	std::array<char, 128> buffer;
	std::string result;
	
	FILE* pipe = popen(command.c_str(), "r");
	if (!pipe)
	{
		std::filesystem::current_path(original_path);
		Log::CoreError("Failed to execute command: {}", command);
		return false;
	}
	
	while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
	{
		result += buffer.data();
	}
	
	int exit_code = pclose(pipe);
	std::filesystem::current_path(original_path);
	
	if (output)
	{
		*output = result;
	}
	
	return exit_code == 0;
}

bool GitHelper::InitRepository(const std::filesystem::path& directory)
{
	Log::CoreInfo("Initializing Git repository in: {}", directory.string());
	bool result = RunGitCommand("git init", directory);
	
	if (result)
	{
		Log::CoreInfo("Git repository initialized successfully");
	}
	else
	{
		Log::CoreError("Failed to initialize Git repository");
	}
	
	return result;
}

bool GitHelper::AddIgnisSubmodule(
	const std::filesystem::path& project_directory,
	const std::string& ignis_repo_url)
{
	Log::CoreInfo("Adding Ignis as Git submodule from: {}", ignis_repo_url);
	std::string command = "git submodule add " + ignis_repo_url + " Ignis";
	bool result = RunGitCommand(command, project_directory);
	
	if (result)
	{
		Log::CoreInfo("Ignis submodule added successfully");
	}
	else
	{
		Log::CoreError("Failed to add Ignis submodule");
		Log::CoreError("Please check your internet connection and try again");
	}
	
	return result;
}

bool GitHelper::UpdateSubmodules(const std::filesystem::path& project_directory)
{
	Log::CoreInfo("Initializing and updating Git submodules...");
	bool result = RunGitCommand("git submodule update --init --recursive", project_directory);
	
	if (result)
	{
		Log::CoreInfo("Git submodules updated successfully");
	}
	else
	{
		Log::CoreError("Failed to update Git submodules");
	}
	
	return result;
}

} // namespace ignis
