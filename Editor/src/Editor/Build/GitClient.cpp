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

	bool GitClient::AddIgnisSubmodule(
		const std::filesystem::path& projectDirectory,
		const std::string& ignisRepoUrl)
	{
		Log::CoreInfo("Adding Ignis as Git submodule from: {}", ignisRepoUrl);
		auto result = ProcessRunner::RunCommand(
			"git submodule add " + ignisRepoUrl + " Ignis", projectDirectory);

		if (result.Success)
			Log::CoreInfo("Ignis submodule added successfully");
		else
		{
			Log::CoreError("Failed to add Ignis submodule");
			Log::CoreError("Please check your internet connection and try again");
		}

		return result.Success;
	}

	bool GitClient::UpdateSubmodules(const std::filesystem::path& projectDirectory)
	{
		Log::CoreInfo("Initializing and updating Git submodules...");
		auto result = ProcessRunner::RunCommand(
			"git submodule update --init --recursive", projectDirectory);

		if (result.Success)
			Log::CoreInfo("Git submodules updated successfully");
		else
			Log::CoreError("Failed to update Git submodules");

		return result.Success;
	}

} // namespace ignis