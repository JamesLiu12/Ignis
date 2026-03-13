#pragma once

namespace ignis {

class GitClient
{
public:
	// Check if Git is available on the system
	static bool IsGitAvailable();
	
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
