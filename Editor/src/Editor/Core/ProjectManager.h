#pragma once

namespace ignis {

/**
 * @brief Manages all project-related operations
 * 
 * Handles project loading, saving, creation, and closing.
 * Separates logic from UI layer in EditorLayer.
 */
class ProjectManager
{
public:
	/**
	 * @brief Open a project from file path
	 * @param filepath Path to .igproj file
	 */
	static void OpenProject(const std::filesystem::path& filepath);

	/**
	 * @brief Save current project to its current location
	 */
	static void SaveProject();

	/**
	 * @brief Save current project to specific file path
	 * @param filepath Path to save .igproj file
	 */
	static void SaveProject(const std::filesystem::path& filepath);

	/**
	 * @brief Save current project to a new location (copy entire project)
	 * @param destinationFolder Folder where project will be copied
	 * @return true if successful, false otherwise
	 */
	static bool SaveProjectAs(const std::filesystem::path& destinationFolder);

	/**
	 * @brief Close current project (saves before closing)
	 */
	static void CloseProject();

	/**
	 * @brief Create a new project
	 * @param name Project name
	 * @param location Parent directory where project folder will be created
	 * @return true if successful, false otherwise
	 */
	static bool CreateNewProject(const std::string& name, const std::filesystem::path& location);
};

} // namespace ignis
