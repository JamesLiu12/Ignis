#include "ProjectManager.h"
#include "Ignis/Project/Project.h"
#include "Ignis/Project/ProjectSerializer.h"
#include "Ignis/Scene/SceneSerializer.h"
#include "Ignis/Core/Log.h"
#include "Ignis/Core/File/FileSystem.h"
#include "Editor/EditorApp.h"
#include "Editor/EditorSceneLayer.h"
#include "Editor/Panels/AssetBrowserPanel.h"
#include "Editor/Project/TemplateProcessor.h"
#include "Editor/Build/GitClient.h"

namespace ignis {

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
			
			// Save editor scene (not runtime scene, even if in Play mode)
			if (auto* app = dynamic_cast<EditorApp*>(&Application::Get()))
			{
				if (auto* scene_layer = app->GetSceneLayer())
				{
					// Warn if saving during Play mode
					if (scene_layer->GetSceneState() == EditorSceneLayer::SceneState::Play)
					{
						Log::CoreWarn("Saving during Play mode - runtime changes will NOT be saved");
					}
					
					// Always save editor scene, not runtime scene
					if (auto scene = scene_layer->GetEditorScene())
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

					AssetManager::SaveAssetRegistry(project->GetActiveAssetRegistry());
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

	AssetManager::SaveAssetRegistry(project->GetActiveAssetRegistry());

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
		
		// Clear asset browser
		if (auto* asset_browser = app->GetAssetBrowserPanel())
		{
			asset_browser->Clear();
		}
	}

	Log::CoreInfo("Project closed");
}

bool ProjectManager::CreateNewProject(const std::string& name, const std::filesystem::path& location)
{
	std::filesystem::path project_dir = location / name;

	// Check if directory already exists
	if (std::filesystem::exists(project_dir))
	{
		Log::CoreError("Project directory already exists: {}", project_dir.string());
		return false;
	}

	// Create project directory structure
	std::filesystem::create_directories(project_dir);
	std::filesystem::create_directories(project_dir / "assets" / "scenes");
	std::filesystem::create_directories(project_dir / "assets" / "scripts");

	// Check if Git is available
	if (!GitClient::IsGitAvailable())
	{
		Log::CoreError("Git is not available. Please install Git and try again.");
		Log::CoreError("Download Git from: https://git-scm.com/downloads");
		return false;
	}

	// Initialize Git repository
	if (!GitClient::InitRepository(project_dir))
	{
		Log::CoreError("Failed to initialize Git repository");
		return false;
	}

	// Add Ignis as submodule
	if (!GitClient::AddSubmodule(project_dir, "https://github.com/JamesLiu12/Ignis", "Ignis"))
	{
		Log::CoreError("Failed to add Ignis submodule");
		Log::CoreError("Please check your internet connection and try again");
		return false;
	}

	// Update submodules (download Ignis)
	if (!GitClient::UpdateSubmodules(project_dir))
	{
		Log::CoreError("Failed to download Ignis engine");
		return false;
	}

	// Get template directory
	auto template_dir = FileSystem::GetExecutableDirectory() / "resources" / "templates";
	if (!std::filesystem::exists(template_dir))
	{
		Log::CoreError("Template directory not found: {}", template_dir.string());
		return false;
	}

	// Prepare template variables
	std::map<std::string, std::string> variables = {
		{"${PROJECT_NAME}", name},
		{"${PROJECT_NAME_UPPER}", TemplateProcessor::ToUpperCase(name)}
	};

	// Process template files
	if (!TemplateProcessor::ProcessTemplate(
			template_dir / "CMakeLists.txt.template",
			project_dir / "CMakeLists.txt",
			variables))
	{
		Log::CoreError("Failed to create CMakeLists.txt");
		return false;
	}

	if (!TemplateProcessor::ProcessTemplate(
			template_dir / "CMakePresets.json.template",
			project_dir / "CMakePresets.json",
			variables))
	{
		Log::CoreError("Failed to create CMakePresets.json");
		return false;
	}

	if (!TemplateProcessor::ProcessTemplate(
			template_dir / "ScriptModuleExports.cpp.template",
			project_dir / "assets" / "scripts" / "ScriptModuleExports.cpp",
			variables))
	{
		Log::CoreError("Failed to create ScriptModuleExports.cpp");
		return false;
	}

	if (!TemplateProcessor::ProcessTemplate(
			template_dir / ".gitignore.template",
			project_dir / ".gitignore",
			variables))
	{
		Log::CoreError("Failed to create .gitignore");
		return false;
	}

	// Create .igproj file with ScriptModule configuration
	std::filesystem::path project_file = project_dir / (name + ".igproj");
	std::ofstream project_file_stream(project_file);
	if (!project_file_stream.is_open())
	{
		Log::CoreError("Failed to create project file: {}", project_file.string());
		return false;
	}

	project_file_stream << "{\n";
	project_file_stream << "  \"ProjectName\": \"" << name << "\",\n";
	project_file_stream << "  \"AssetDirectory\": \"assets/\",\n";
	project_file_stream << "  \"AssetRegistry\": \"assets/AssetRegistry.igar\",\n";
	project_file_stream << "  \"StartScene\": \"scenes/MainScene.igscene\",\n";
	project_file_stream << "  \"ScriptModule\": {\n";
	project_file_stream << "    \"Name\": \"" << name << "\",\n";
	project_file_stream << "    \"Directory\": \"bin/{Platform}/{Config}/\",\n";
	project_file_stream << "    \"Windows\": \"{Name}.dll\",\n";
	project_file_stream << "    \"Linux\": \"lib{Name}.so\",\n";
	project_file_stream << "    \"macOS\": \"lib{Name}.dylib\"\n";
	project_file_stream << "  }\n";
	project_file_stream << "}\n";
	project_file_stream.close();

	// Create empty asset registry
	std::ofstream asset_registry(project_dir / "assets" / "AssetRegistry.igar");
	asset_registry << "{\n";
	asset_registry << "  \"Assets\": []\n";
	asset_registry << "}";
	asset_registry.close();

	// Create empty scene
	std::ofstream scene(project_dir / "assets" / "scenes" / "MainScene.igscene");
	scene << "{\n";
	scene << "  \"Scene\": \"MainScene\",\n";
	scene << "  \"Entities\": []\n";
	scene << "}";
	scene.close();

	// Log success and instructions
	Log::CoreInfo("=== New Project Created Successfully ===");
	Log::CoreInfo("Project: {}", name);
	Log::CoreInfo("Location: {}", project_dir.string());
	Log::CoreInfo("");
	Log::CoreInfo("Next steps:");
	Log::CoreInfo("1. Create your scene and add scripts in the Editor");
	Log::CoreInfo("2. Build your scripts before exporting:");
	Log::CoreInfo("");

#if defined(__APPLE__)
	Log::CoreInfo("   Open Terminal and run:");
	Log::CoreInfo("   cd \"{}\"", project_dir.string());
	Log::CoreInfo("   cmake --preset arm64-debug");
	Log::CoreInfo("   cmake --build --preset arm64-debug");
#elif defined(_WIN32)
	Log::CoreInfo("   Open Command Prompt and run:");
	Log::CoreInfo("   cd \"{}\"", project_dir.string());
	Log::CoreInfo("   cmake --preset x64-debug");
	Log::CoreInfo("   cmake --build --preset x64-debug");
#else
	Log::CoreInfo("   Open Terminal and run:");
	Log::CoreInfo("   cd \"{}\"", project_dir.string());
	Log::CoreInfo("   cmake -B build -DCMAKE_BUILD_TYPE=Debug");
	Log::CoreInfo("   cmake --build build");
#endif

	Log::CoreInfo("");
	Log::CoreInfo("3. After building, export your game from Project -> Export Game");
	Log::CoreInfo("========================================");

	// Open the new project
	OpenProject(project_file);
	return true;
}

} // namespace ignis
