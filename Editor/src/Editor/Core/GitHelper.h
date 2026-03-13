#pragma once

namespace ignis {

class GitHelper
{
public:
	// Check if Git is available on the system
	static bool IsGitAvailable();
	
	// Execute a Git command in the specified directory
	// Returns true if command succeeded (exit code 0)
	static bool RunGitCommand(
		const std::string& command,
		const std::filesystem::path& working_directory,
		std::string* output = nullptr
	);
	
	// Initialize a new Git repository
	static bool InitRepository(const std::filesystem::path& directory);
	
	// Add Ignis as a submodule
	static bool AddIgnisSubmodule(
		const std::filesystem::path& project_directory,
		const std::string& ignis_repo_url = "https://github.com/JamesLiu12/Ignis"
	);
	
	// Initialize and update submodules
	static bool UpdateSubmodules(const std::filesystem::path& project_directory);
};

} // namespace ignis
