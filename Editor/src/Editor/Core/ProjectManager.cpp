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
	// Auto-save current project before switching
	if (Project::GetActive())
	{
		SaveProject();
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
	if (!Project::GetActive())
	{
		Log::CoreWarn("No active project to close");
		return;
	}

	// Auto-save before closing
	SaveProject();

	// Clear active project
	Project::SetActive(nullptr);

	// Clear scene in editor
	if (auto* app = dynamic_cast<EditorApp*>(&Application::Get()))
	{
		if (auto* scene_layer = app->GetSceneLayer())
		{
			scene_layer->ClearProject();
		}
	}

	Log::CoreInfo("Project closed");
}

bool ProjectManager::CreateNewProject(const std::string& name, const std::filesystem::path& location)
{
	std::filesystem::path projectDir = location / name;

	// Check if directory already exists
	if (std::filesystem::exists(projectDir))
	{
		Log::CoreError("Project directory already exists: {}", projectDir.string());
		return false;
	}

	// Create project directory structure
	std::filesystem::create_directories(projectDir);
	std::filesystem::create_directories(projectDir / "assets");
	std::filesystem::create_directories(projectDir / "assets" / "scenes");

	// Create project file
	std::filesystem::path projectFile = projectDir / (name + ".igproj");

	// Create basic project JSON manually
	std::ofstream projectFileStream(projectFile);
	if (!projectFileStream.is_open())
	{
		Log::CoreError("Failed to create project file: {}", projectFile.string());
		return false;
	}

	projectFileStream << "{\n";
	projectFileStream << "  \"ProjectName\": \"" << name << "\",\n";
	projectFileStream << "  \"AssetDirectory\": \"assets\",\n";
	projectFileStream << "  \"AssetRegistry\": \"assets/AssetRegistry.igar\",\n";
	projectFileStream << "  \"StartScene\": \"scenes/MainScene.igscene\"\n";
	projectFileStream << "}\n";
	projectFileStream.close();

	// Create empty asset registry with proper structure
	std::ofstream assetRegistry(projectDir / "assets" / "AssetRegistry.igar");
	assetRegistry << "{\n";
	assetRegistry << "  \"Assets\": []\n";
	assetRegistry << "}";
	assetRegistry.close();

	// Create empty scene with proper structure
	std::ofstream scene(projectDir / "assets" / "scenes" / "MainScene.igscene");
	scene << "{\n";
	scene << "  \"Scene\": \"MainScene\",\n";
	scene << "  \"Entities\": []\n";
	scene << "}";
	scene.close();

	Log::CoreInfo("Created new project: {}", projectFile.string());

	// Open the new project
	OpenProject(projectFile);
	return true;
}

} // namespace ignis
