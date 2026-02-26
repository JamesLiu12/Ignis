#include "ProjectManager.h"
#include "Ignis/Project/Project.h"
#include "Ignis/Project/ProjectSerializer.h"
#include "Ignis/Scene/SceneSerializer.h"
#include "Ignis/Core/Log.h"
#include "Editor/EditorApp.h"
#include "Editor/EditorSceneLayer.h"

namespace ignis {

std::optional<std::filesystem::path> ProjectManager::FindProjectFile(const std::filesystem::path& folder)
{
	std::string projectName = folder.filename().string();
	std::filesystem::path projectFile = folder / (projectName + ".igproj");
	
	if (std::filesystem::exists(projectFile))
	{
		return projectFile;
	}
	
	Log::CoreError("No .igproj file found in folder: {}", folder.string());
	return std::nullopt;
}

void ProjectManager::OpenProject(const std::filesystem::path& filepath)
{
	// Unmount previous project's assets if any project was active
	if (Project::GetActive())
	{
		VFS::Unmount("assets");
	}
	
	ProjectSerializer project_serializer;

	if (auto project = project_serializer.Deserialize(filepath))
	{
		Project::SetActive(project);
		
		// Reload scene in editor
		if (auto* app = dynamic_cast<EditorApp*>(&Application::Get()))
		{
			if (auto* scene_layer = app->GetSceneLayer())
			{
				scene_layer->ReloadProject();
			}
		}
		
		Log::CoreInfo("Project loaded: {}", filepath.string());
	}
	else
	{
		Log::CoreError("Failed to open project: {}", filepath.string());
	}
}

void ProjectManager::SaveProject()
{
	auto project = Project::GetActive();
	if (!project)
	{
		Log::CoreError("No active project to save");
		return;
	}

	// Save to current project path
	std::filesystem::path projectPath = project->GetProjectDirectory() / 
	                                     (project->GetProjectName() + ".igproj");
	SaveProject(projectPath);
}

void ProjectManager::SaveProject(const std::filesystem::path& filepath)
{
	ProjectSerializer project_serializer;

	if (auto project = Project::GetActive())
	{
		// Save project metadata
		if (project_serializer.Serialize(*project, filepath))
		{
			Log::CoreInfo("Project saved: {}", filepath.string());
			
			// Save active scene
			if (auto* app = dynamic_cast<EditorApp*>(&Application::Get()))
			{
				if (auto* scene_layer = app->GetSceneLayer())
				{
					if (auto scene = scene_layer->GetScene())
					{
						SceneSerializer scene_serializer;
						if (scene_serializer.Serialize(*scene, Project::GetActiveStartScene()))
						{
							Log::CoreInfo("Scene saved: {}", Project::GetActiveStartScene().string());
						}
						else
						{
							Log::CoreError("Failed to save scene: {}", Project::GetActiveStartScene().string());
						}
					}
				}
			}
		}
		else
		{
			Log::CoreError("Failed to save project: {}", filepath.string());
		}
	}
	else
	{
		Log::CoreError("No active project to save");
	}
}

bool ProjectManager::SaveProjectAs(const std::filesystem::path& destinationFolder)
{
	auto project = Project::GetActive();
	if (!project)
	{
		Log::CoreError("No active project to save");
		return false;
	}

	// Create new project directory path
	std::filesystem::path newProjectDir = destinationFolder / project->GetProjectName();
	
	// Check if directory already exists
	if (std::filesystem::exists(newProjectDir))
	{
		Log::CoreError("Project directory already exists: {}", newProjectDir.string());
		return false;
	}

	// Create new project directory
	std::filesystem::create_directories(newProjectDir);
	
	// Copy assets folder
	std::filesystem::path oldAssetsDir = project->GetProjectDirectory() / "assets";
	std::filesystem::path newAssetsDir = newProjectDir / "assets";
	if (std::filesystem::exists(oldAssetsDir))
	{
		std::filesystem::copy(oldAssetsDir, newAssetsDir, 
		                     std::filesystem::copy_options::recursive);
	}

	// Save project file to new location
	std::filesystem::path newProjectFile = newProjectDir / (project->GetProjectName() + ".igproj");
	
	ProjectSerializer serializer;
	if (serializer.Serialize(*project, newProjectFile))
	{
		Log::CoreInfo("Project saved to new location: {}", newProjectFile.string());
		// Reload project from new location
		OpenProject(newProjectFile);
		return true;
	}
	else
	{
		Log::CoreError("Failed to save project to new location: {}", newProjectFile.string());
		return false;
	}
}

void ProjectManager::CloseProject()
{
	// TODO: Phase 3 - Save and unload current project
	Log::CoreInfo("CloseProject() - Not yet implemented");
}

bool ProjectManager::CreateNewProject(const std::string& name, const std::filesystem::path& location)
{
	// TODO: Phase 3 - Implement new project creation
	Log::CoreInfo("CreateNewProject() - Not yet implemented");
	return false;
}

} // namespace ignis
