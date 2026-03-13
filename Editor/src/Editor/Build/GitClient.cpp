#include "GitClient.h"
#include "Editor/Core/ProcessRunner.h"
#include "Ignis/Core/Log.h"

namespace ignis {

	bool GitClient::IsGitAvailable()
	{
		auto result = ProcessRunner::RunCommand("git --version");
		if (result.Success)
			Log::CoreInfo("Git is available: {}", result.Output);
		return result.Success;
	}

	bool GitClient::InitRepository(const std::filesystem::path& directory)
	{
		Log::CoreInfo("Initializing Git repository in: {}", directory.string());
		auto result = ProcessRunner::RunCommand("git init", directory);

		if (result.Success)
			Log::CoreInfo("Git repository initialized successfully");
		else
			Log::CoreError("Failed to initialize Git repository");

		return result.Success;
	}

	bool GitClient::AddSubmodule(
		const std::filesystem::path& project_directory,
		const std::string& repo_url,
		const std::string& local_path)
	{
		Log::CoreInfo("Adding submodule '{}' from: {}", local_path, repo_url);
		auto result = ProcessRunner::RunCommand(
			"git submodule add " + repo_url + " " + local_path, project_directory);

		if (result.Success)
			Log::CoreInfo("Submodule '{}' added successfully", local_path);
		else
			Log::CoreError("Failed to add submodule '{}' from: {}", local_path, repo_url);

		return result.Success;
	}

	bool GitClient::UpdateSubmodules(const std::filesystem::path& project_directory)
	{
		Log::CoreInfo("Initializing and updating Git submodules...");
		auto result = ProcessRunner::RunCommand(
			"git submodule update --init --recursive", project_directory);

		if (result.Success)
			Log::CoreInfo("Git submodules updated successfully");
		else
			Log::CoreError("Failed to update Git submodules");

		return result.Success;
	}

} // namespace ignis