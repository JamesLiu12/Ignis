#include "Editor/EditorLayer.h"
#include "Editor/Project/ProjectManager.h"
#include "Editor/Build/CMakeBuilder.h"

#include "Ignis/Core/File/FileDialog.h"
#include "Ignis/Project/Project.h"
#include "Ignis/Project/ProjectSerializer.h"
#include "Ignis/Scene/Scene.h"
#include "Ignis/Scene/SceneSerializer.h"
#include "Ignis/Asset/AssetManager.h"
#include "Editor/EditorApp.h"
#include "Editor/EditorSceneLayer.h"

#include <imgui.h>

namespace ignis 
{

	// Initialize static buffers
	char EditorLayer::s_OpenProjectFilePathBuffer[512] = "";
	char EditorLayer::s_SaveProjectAsFolderBuffer[512] = "";
	char EditorLayer::s_NewProjectFolderBuffer[512] = "";
	char EditorLayer::s_NewProjectNameBuffer[128] = "";
	bool EditorLayer::s_ShowNewProjectPopup = false;

	// Scene management static buffers
	char EditorLayer::s_LoadSceneFilePathBuffer[512] = "";
	char EditorLayer::s_SaveSceneAsFolderBuffer[512] = "";
	char EditorLayer::s_NewSceneFolderBuffer[512] = "";
	char EditorLayer::s_NewSceneNameBuffer[128] = "";
	bool EditorLayer::s_ShowNewScenePopup = false;

	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
	}

	void EditorLayer::OnAttach()
	{
		Log::CoreInfo("EditorLayer attached");

		// Create panel manager
		m_panel_manager = std::make_unique<PanelManager>();
		
		Log::CoreInfo("EditorLayer initialized with PanelManager");
		VFS::Mount("resources", "resources");
		
	}

	void EditorLayer::OnDetach()
	{
		// Put your test project here
		// Don't log here - logging system may already be shut down during application destruction
	}

	void EditorLayer::OnUpdate(float dt)
	{

	}

	void EditorLayer::OnImGuiRender()
	{
		// Handle keyboard shortcuts first
		HandleKeyboardShortcuts();
		
		// Render menu bar
		RenderMenuBar();

		// Render all panels managed by PanelManager
		m_panel_manager->OnImGuiRender();

		// Handle new project popup
		if (s_ShowNewProjectPopup)
		{
			UI_ShowNewProjectPopup();
		}

		// Handle new scene popup
		if (s_ShowNewScenePopup)
		{
			UI_ShowNewScenePopup();
		}

		// Process deferred operations (safe after ImGui rendering)
		ProcessDeferredProjectLoad();
		ProcessDeferredSaveAs();
		ProcessDeferredSceneLoad();
		ProcessDeferredSceneSaveAs();
	}

	void EditorLayer::OnEvent(EventBase& event)
	{
		// Forward events to all panels
		m_panel_manager->OnEvent(event);
	}

	void EditorLayer::OpenProject()
	{
		// Select .igproj file with filter
		std::string projectFile = FileDialog::OpenFile("Ignis Project", {"igproj"});
		if (projectFile.empty())
			return;

		// Defer actual loading
		strcpy(s_OpenProjectFilePathBuffer, projectFile.c_str());
	}

	void EditorLayer::OpenProject(const std::filesystem::path& filepath)
	{
		ProjectManager::OpenProject(filepath);
	}
	
	void EditorLayer::SaveProject()
	{
		ProjectManager::SaveProject();
	}

	void EditorLayer::SaveProject(const std::filesystem::path& filepath)
	{
		ProjectManager::SaveProject(filepath);
	}

	void EditorLayer::SaveProjectAs()
	{
		auto project = Project::GetActive();
		if (!project)
		{
			Log::CoreError("No active project to save");
			return;
		}

		// Open folder dialog to select new location
		std::string folder = FileDialog::OpenFolder();
		if (folder.empty())
			return;

		// Defer actual save operation
		strcpy(s_SaveProjectAsFolderBuffer, folder.c_str());
	}

	void EditorLayer::CloseProject()
	{
		ProjectManager::CloseProject();
	}

	void EditorLayer::ProcessDeferredProjectLoad()
	{
		if (strlen(s_OpenProjectFilePathBuffer) > 0)
		{
			OpenProject(s_OpenProjectFilePathBuffer);
			memset(s_OpenProjectFilePathBuffer, 0, sizeof(s_OpenProjectFilePathBuffer));
		}
	}

	void EditorLayer::ProcessDeferredSaveAs()
	{
		if (strlen(s_SaveProjectAsFolderBuffer) > 0)
		{
			ProjectManager::SaveProjectAs(s_SaveProjectAsFolderBuffer);
			memset(s_SaveProjectAsFolderBuffer, 0, sizeof(s_SaveProjectAsFolderBuffer));
		}
	}

	void EditorLayer::CreateNewProject()
	{
		// Show new project popup
		s_ShowNewProjectPopup = true;
	}

	void EditorLayer::HandleKeyboardShortcuts()
	{
		ImGuiIO& io = ImGui::GetIO();
		
		// Don't process shortcuts when typing in text fields
		if (io.WantTextInput)
			return;
		
		// Cmd/Ctrl + O: Open Project
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O))
		{
			OpenProject();
		}
		
		// Cmd/Ctrl + S: Save Project
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S) && !io.KeyShift)
		{
			if (Project::GetActive())
				SaveProject();
		}
		
		// Cmd/Ctrl + Shift + S: Save Project As
		if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_S))
		{
			if (Project::GetActive())
				SaveProjectAs();
		}

		// Cmd/Ctrl + B: Build Scripts
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_B))
		{
			if (Project::GetActive())
				BuildScripts();
		}

		// Cmd/Ctrl + E: Export Game
		if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_E))
		{
			if (Project::GetActive())
				ExportGame();
		}
	}

	void EditorLayer::UI_ShowNewProjectPopup()
	{
		// TODO: - Implement new project creation popup
		ImGui::OpenPopup("New Project");

		if (ImGui::BeginPopupModal("New Project", &s_ShowNewProjectPopup, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Create a new Ignis project");
			ImGui::Separator();

			ImGui::InputText("Project Name", s_NewProjectNameBuffer, sizeof(s_NewProjectNameBuffer));
			ImGui::InputText("Location", s_NewProjectFolderBuffer, sizeof(s_NewProjectFolderBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse..."))
			{
				std::string folder = FileDialog::OpenFolder();
				if (!folder.empty())
				{
					strcpy(s_NewProjectFolderBuffer, folder.c_str());
				}
			}

			ImGui::Separator();

			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				if (strlen(s_NewProjectNameBuffer) > 0 && strlen(s_NewProjectFolderBuffer) > 0)
				{
					ProjectManager::CreateNewProject(s_NewProjectNameBuffer, s_NewProjectFolderBuffer);

					// Clear buffers
					memset(s_NewProjectNameBuffer, 0, sizeof(s_NewProjectNameBuffer));
					memset(s_NewProjectFolderBuffer, 0, sizeof(s_NewProjectFolderBuffer));
				}
				s_ShowNewProjectPopup = false;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				s_ShowNewProjectPopup = false;
			}

			ImGui::EndPopup();
		}
	}

	void EditorLayer::RenderMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			// Project Menu
			if (ImGui::BeginMenu("Project"))
			{
				if (ImGui::MenuItem("New Project"))
				{
					CreateNewProject();
				}

				// Platform-specific shortcut text
				#ifdef __APPLE__
					const char* openShortcut = "Cmd+O";
					const char* saveShortcut = "Cmd+S";
				#else
					const char* openShortcut = "Ctrl+O";
					const char* saveShortcut = "Ctrl+S";
				#endif

				if (ImGui::MenuItem("Load Project", openShortcut))
				{
					OpenProject();
				}

				bool hasProject = Project::GetActive() != nullptr;
				if (ImGui::MenuItem("Save Project", saveShortcut, false, hasProject))
				{
					SaveProject();
				}

				#ifdef __APPLE__
					const char* saveAsShortcut = "Cmd+Shift+S";
				#else
					const char* saveAsShortcut = "Ctrl+Shift+S";
				#endif
				if (ImGui::MenuItem("Save Project As", saveAsShortcut, false, hasProject))
				{
					SaveProjectAs();
				}

				if (ImGui::MenuItem("Close Project", nullptr, false, hasProject))
				{
					CloseProject();
				}

				ImGui::Separator();

				#ifdef __APPLE__
					const char* buildShortcut = "Cmd+B";
				#else
					const char* buildShortcut = "Ctrl+B";
				#endif
				if (ImGui::MenuItem("Build Scripts", buildShortcut, false, hasProject))
				{
					BuildScripts();
				}

				#ifdef __APPLE__
					const char* exportShortcut = "Cmd+E";
				#else
					const char* exportShortcut = "Ctrl+E";
				#endif
				if (ImGui::MenuItem("Export Game", exportShortcut, false, hasProject))
				{
					ExportGame();
				}

				ImGui::EndMenu();
			}

			// Scene Menu
			if (ImGui::BeginMenu("Scene"))
			{
				bool hasProject = Project::GetActive() != nullptr;

				if (ImGui::MenuItem("New Scene", nullptr, false, hasProject))
				{
					NewScene();
				}

				if (ImGui::MenuItem("Load Scene", nullptr, false, hasProject))
				{
					LoadScene();
				}

				if (ImGui::MenuItem("Save Scene", nullptr, false, hasProject))
				{
					SaveScene();
				}

				if (ImGui::MenuItem("Save Scene As", nullptr, false, hasProject))
				{
					SaveSceneAs();
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Set Current Scene As Start Scene", nullptr, false, hasProject))
				{
					SetCurrentSceneAsStartScene();
				}

				ImGui::EndMenu();
			}

			// View Menu
			if (ImGui::BeginMenu("View"))
			{
				// Toggle panels on/off
				auto& panels = m_panel_manager->GetPanels();
				for (auto& panel_data : panels)
				{
					ImGui::MenuItem(panel_data.Name.c_str(), nullptr, &panel_data.IsOpen);
				}
				ImGui::EndMenu();
			}

			// Help Menu
			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("About Ignis Editor"))
				{
					Log::CoreInfo("Ignis Editor - Game Engine Editor");
				}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

	void EditorLayer::BuildScripts()
	{
		auto project = Project::GetActive();
		if (!project)
		{
			Log::CoreError("No active project to build");
			return;
		}

		CMakeBuilder::BuildOptions options;
		options.SourceDir = Project::GetActiveProjectDirectory();
		options.BuildDir = options.SourceDir / "out" / "build";

		#if defined(_DEBUG)
			options.BuildType = CMakeBuilder::BuildType::Debug;
		#else
			options.BuildType = CMakeBuilder::BuildType::Release;
		#endif

		#if defined(_WIN32)
			options.Arch = CMakeBuilder::VSArch::X64;
		#elif defined(__APPLE__)
			options.Arch = CMakeBuilder::VSArch::ARM64;
			options.ExtraArgs.push_back("-DCMAKE_OSX_ARCHITECTURES=arm64");
		#endif

		if (options.Arch == CMakeBuilder::VSArch::X64)
		{
			if (options.BuildType == CMakeBuilder::BuildType::Debug)
				options.BuildDir /= "x64-debug";
			else
				options.BuildDir /= "x64-release";
		}
		else if (options.Arch == CMakeBuilder::VSArch::ARM64)
		{
			if (options.BuildType == CMakeBuilder::BuildType::Debug)
				options.BuildDir /= "arm64-debug";
			else
				options.BuildDir /= "arm64-release";
		}
		else
		{
			Log::CoreError("Unsupported Platform");
			return;
		}

		Log::CoreInfo("=== Building Scripts ===");
		Log::CoreInfo("Project: {}", Project::GetActiveProjectName());

		if (!CMakeBuilder::IsCMakeAvailable())
		{
			Log::CoreError("CMake not found, cannot build scripts");
			return;
		}

		if (CMakeBuilder::ConfigureAndBuild(options))
		{
			Log::CoreInfo("=== Scripts Built Successfully ===");
			EditorApp& editor_app = static_cast<EditorApp&>(Application::Get());
			editor_app.GetSceneLayer()->OnScriptsReload();
		}
		else
		{
			Log::CoreError("=== Scripts Build Failed ===");
		}
	}

	void EditorLayer::ExportGame()
	{
		if (!Project::GetActive())
		{
			Log::CoreError("No active project to export");
			return;
		}

		auto project_name = Project::GetActiveProjectName();
		auto project_dir = Project::GetActiveProjectDirectory();

		Log::CoreInfo("=== Starting Game Export ===");
		Log::CoreInfo("Project: {}", project_name);

		// Helper lambda to replace tokens in paths
		auto ReplaceTokens = [](std::string str, const std::string& platform, const std::string& config, const std::string& name) -> std::string {
			size_t pos = 0;
			while ((pos = str.find("{Platform}", pos)) != std::string::npos) {
				str.replace(pos, 10, platform);
				pos += platform.length();
			}
			pos = 0;
			while ((pos = str.find("{Config}", pos)) != std::string::npos) {
				str.replace(pos, 8, config);
				pos += config.length();
			}
			pos = 0;
			while ((pos = str.find("{Name}", pos)) != std::string::npos) {
				str.replace(pos, 6, name);
				pos += name.length();
			}
			return str;
		};
	
		// Get platform and config using macros
		std::string platform;
		#if defined(_WIN32)
			platform = "Windows";
		#elif defined(__APPLE__)
			platform = "macOS";
		#elif defined(__linux__)
			platform = "Linux";
		#endif
	
		std::string config;
		#if defined(_DEBUG)
			config = "Debug";
		#else
			config = "Release";
		#endif
	
		// Construct build directory path using tokens
		std::string build_dir_pattern;
		#if defined(_WIN32)
			build_dir_pattern = "out/build/x64-{Config}";
		#elif defined(__APPLE__)
			build_dir_pattern = "out/build/arm64-{Config}";
		#else
			build_dir_pattern = "out/build/{Config}";
		#endif
	
		std::string build_dir_str = ReplaceTokens(build_dir_pattern, platform, config, project_name);
		auto build_dir = project_dir / build_dir_str;
		auto runtime_bin_dir = build_dir / "bin";
	
		// Runtime executable is always named "IgnisRuntime"
		// Export will copy and rename it to project name
		std::string runtime_exe_name;
		#if defined(_WIN32)
			runtime_exe_name = "IgnisRuntime.exe";
		#else
			runtime_exe_name = "IgnisRuntime";
		#endif
	
		// Validate that Runtime executable exists (should be pre-built)
		auto runtime_exe = runtime_bin_dir / runtime_exe_name;
		if (!std::filesystem::exists(runtime_exe))
		{
			Log::CoreError("Runtime executable not found: {}", runtime_exe.string());
			Log::CoreError("Please build the project first before exporting.");
			#if defined(_WIN32)
				if (config == "Debug")
					Log::CoreError("Run: cmake --build --preset x64-debug");
				else
					Log::CoreError("Run: cmake --build --preset x64-release");
			#elif defined(__APPLE__)
				if (config == "Debug")
					Log::CoreError("Run: cmake --build --preset arm64-debug");
				else
					Log::CoreError("Run: cmake --build --preset arm64-release");
			#else
				// Linux or fallback
				std::string build_cmd = "cmake --build " + build_dir_str + " --config " + config;
				Log::CoreError("Run: {}", build_cmd);
			#endif
			return;
		}
	
		// Construct script module name using tokens (same pattern as project config)
		std::string script_module_pattern;
		#if defined(_WIN32)
			script_module_pattern = "{Name}.dll";
		#elif defined(__APPLE__)
			script_module_pattern = "lib{Name}.dylib";
		#else
			script_module_pattern = "lib{Name}.so";
		#endif
		std::string script_module_name = ReplaceTokens(script_module_pattern, platform, config, project_name);
	
		// Script module path: bin/{Platform}/{Config}/
		std::string script_dir_pattern = "bin/{Platform}/{Config}";
		std::string script_dir_str = ReplaceTokens(script_dir_pattern, platform, config, project_name);
		auto script_module_path = project_dir / script_dir_str / script_module_name;
	
		if (!std::filesystem::exists(script_module_path))
		{
			Log::CoreError("=== Script Module Not Built ===");
			Log::CoreError("Script module not found: {}", script_module_path.string());
			Log::CoreError("");
			Log::CoreError("You need to build your project scripts first.");
			Log::CoreError("Open a terminal in your project directory and run:");
			Log::CoreError("");
		
			#if defined(__APPLE__)
				Log::CoreError("  cd \"{}\"", project_dir.string());
				if (config == "Debug")
				{
					Log::CoreError("  cmake --preset arm64-debug");
					Log::CoreError("  cmake --build --preset arm64-debug");
				}
				else
				{
					Log::CoreError("  cmake --preset arm64-release");
					Log::CoreError("  cmake --build --preset arm64-release");
				}
			#elif defined(_WIN32)
				Log::CoreError("  cd \"{}\"", project_dir.string());
				if (config == "Debug")
				{
					Log::CoreError("  cmake --preset x64-debug");
					Log::CoreError("  cmake --build --preset x64-debug");
				}
				else
				{
					Log::CoreError("  cmake --preset x64-release");
					Log::CoreError("  cmake --build --preset x64-release");
				}
			#else
				Log::CoreError("  cd \"{}\"", project_dir.string());
				Log::CoreError("  cmake -B build -DCMAKE_BUILD_TYPE={}", config);
				Log::CoreError("  cmake --build build");
			#endif
		
			Log::CoreError("");
			Log::CoreError("After building, try exporting again.");
			Log::CoreError("===============================");
			return;
		}
	
		Log::CoreInfo("Pre-built binaries validated successfully");

		// Create distribution folder
		auto export_dir = project_dir / "Exports";
		auto dist_dir = export_dir / (project_name + "_Distribution");
	
		Log::CoreInfo("Packaging to: {}", dist_dir.string());
	
		std::filesystem::create_directories(dist_dir);
		std::filesystem::create_directories(dist_dir / "scripts");

		try
		{
			// Copy runtime executable and rename to project name
			std::string dist_exe_name;
			#if defined(_WIN32)
				dist_exe_name = project_name + ".exe";
			#else
				dist_exe_name = project_name;
			#endif
		
			std::filesystem::copy_file(runtime_exe, dist_dir / dist_exe_name,
				std::filesystem::copy_options::overwrite_existing);
			Log::CoreInfo("Copied runtime executable: {} -> {}", runtime_exe_name, dist_exe_name);
		
			// Copy engine and dependency DLLs
			for (const auto& entry : std::filesystem::directory_iterator(runtime_bin_dir))
			{
				if (!entry.is_regular_file()) continue;
			
				auto ext = entry.path().extension();
				auto filename = entry.path().filename();
			
				// Check if it's a shared library (but not the script module)
				if (ext == ".dylib" || ext == ".dll" || ext == ".so")
				{
					// Skip script module - we'll copy it separately
					if (filename.stem().string().find(project_name) == std::string::npos)
					{
						std::filesystem::copy_file(entry.path(),
							dist_dir / filename,
							std::filesystem::copy_options::overwrite_existing);
						Log::CoreInfo("Copied: {}", filename.string());
					}
				}
			}
		
			// Copy script module DLL from project bin directory
			std::filesystem::copy_file(script_module_path,
				dist_dir / "scripts" / script_module_name,
				std::filesystem::copy_options::overwrite_existing);
			Log::CoreInfo("Copied to scripts/: {}", script_module_name);
		
			// Copy and update project file for distribution
			auto project_file = project_dir / (project_name + ".igproj");
			auto dist_project_file = dist_dir / (project_name + ".igproj");
		
			// Load project, update script module path, and save to distribution
			ProjectSerializer serializer;
			auto project = serializer.Deserialize(project_file);
			if (project)
			{
				// Update script module directory for distribution (scripts/ instead of bin/Platform/Config/)
				project->SetScriptModuleDirectory("scripts/");
				serializer.Serialize(*project, dist_project_file);
				Log::CoreInfo("Copied and updated: {}.igproj", project_name);
			}
			else
			{
				// Fallback: just copy the file
				std::filesystem::copy_file(project_file, dist_project_file,
					std::filesystem::copy_options::overwrite_existing);
				Log::CoreWarn("Could not update project file, copied as-is: {}.igproj", project_name);
			}
		
			// Copy assets
			auto assets_src = Project::GetActiveAssetDirectory();
			auto assets_dst = dist_dir / "assets";
		
			std::filesystem::copy(assets_src, assets_dst,
				std::filesystem::copy_options::recursive |
				std::filesystem::copy_options::overwrite_existing);
			Log::CoreInfo("Copied: assets/");
		
			// Copy resources (shaders, etc.)
			auto resources_src = runtime_bin_dir / "resources";
			auto resources_dst = dist_dir / "resources";
		
			if (std::filesystem::exists(resources_src))
			{
				std::filesystem::copy(resources_src, resources_dst,
					std::filesystem::copy_options::recursive |
					std::filesystem::copy_options::overwrite_existing);
				Log::CoreInfo("Copied: resources/");
			}
			else
			{
				Log::CoreWarn("Resources folder not found at: {}", resources_src.string());
			}
		
			Log::CoreInfo("=== Export Complete ===");
			Log::CoreInfo("Distribution folder: {}", dist_dir.string());
		
			// Open folder for user
			FileDialog::RevealInFileExplorer(dist_dir);
		}
		catch (const std::exception& e)
		{
			Log::CoreError("Export failed: {}", e.what());
		}
	}

	void EditorLayer::NewScene()
	{
		Log::CoreInfo("New Scene menu item clicked");
		s_ShowNewScenePopup = true;
		
		// Clear buffers (no default values)
		s_NewSceneFolderBuffer[0] = '\0';
		s_NewSceneNameBuffer[0] = '\0';
	}

	void EditorLayer::LoadScene()
	{
		Log::CoreInfo("Load Scene menu item clicked");
		
		// Open file dialog to select .igscene file
		std::string filepath = FileDialog::OpenFile("Ignis Scene", {"igscene"});
		
		if (!filepath.empty())
		{
			// Verify the selected file is within the project assets directory
			std::filesystem::path assets_dir = Project::GetActiveAssetDirectory();
			std::filesystem::path selected_path(filepath);
		
			// Normalize paths for comparison
			std::string assets_str = std::filesystem::canonical(assets_dir).string();
			std::string selected_str = std::filesystem::canonical(selected_path.parent_path()).string();
		
			if (selected_str.find(assets_str) == 0 || selected_str == assets_str)
			{
				std::strncpy(s_LoadSceneFilePathBuffer, filepath.c_str(), sizeof(s_LoadSceneFilePathBuffer) - 1);
				s_LoadSceneFilePathBuffer[sizeof(s_LoadSceneFilePathBuffer) - 1] = '\0';
				Log::CoreInfo("Scene file selected: {}", filepath);
			}
			else
			{
				Log::CoreError("Scene file must be within the project assets folder: {}", assets_dir.string());
			}
		}
	}

	void EditorLayer::SaveScene()
	{
		if (auto* app = dynamic_cast<EditorApp*>(&Application::Get()))
		{
			if (auto* scene_layer = app->GetSceneLayer())
			{
				scene_layer->SaveCurrentScene();
			}
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		// Open file save dialog
		std::string file_path = FileDialog::SaveFile();
		
		if (!file_path.empty())
		{
			std::filesystem::path scene_path(file_path);
			
			// Ensure .igscene extension
			if (scene_path.extension() != ".igscene")
			{
				scene_path += ".igscene";
			}
			
			// Validate that the path is within the project assets directory
			if (!Project::GetActive())
			{
				Log::CoreError("Cannot save scene: No project is loaded");
				return;
			}
			
			std::filesystem::path assets_dir = Project::GetActiveAssetDirectory();
			std::filesystem::path canonical_scene = std::filesystem::weakly_canonical(scene_path);
			std::filesystem::path canonical_assets = std::filesystem::weakly_canonical(assets_dir);
			
			std::string scene_str = canonical_scene.string();
			std::string assets_str = canonical_assets.string();
			
			if (scene_str.find(assets_str) != 0)
			{
				Log::CoreError("Scene must be saved within the project assets folder");
				return;
			}
			
			// Get scene layer and save
			if (auto* app = dynamic_cast<EditorApp*>(&Application::Get()))
			{
				if (auto* scene_layer = app->GetSceneLayer())
				{
					auto editor_scene = scene_layer->GetEditorScene();
					if (!editor_scene)
					{
						Log::CoreError("Cannot save scene: No scene is loaded");
						return;
					}
					
					// Create directory if it doesn't exist
					std::filesystem::path parent_dir = scene_path.parent_path();
					if (!std::filesystem::exists(parent_dir))
					{
						std::filesystem::create_directories(parent_dir);
					}
					
					// Save the editor scene to the new path
					SceneSerializer serializer;
					if (serializer.Serialize(*editor_scene, scene_path))
					{
						Log::CoreInfo("Scene saved as: {}", scene_path.string());
						
						// Register the scene file as an asset
						AssetManager::ImportAsset(scene_path);
						
						// Load the scene at the new path (this updates m_current_scene_path)
						scene_layer->LoadScene(scene_path);
					}
					else
					{
						Log::CoreError("Failed to save scene as: {}", scene_path.string());
					}
				}
			}
		}
	}

	void EditorLayer::SetCurrentSceneAsStartScene()
	{
		Log::CoreInfo("Set Current Scene As Start Scene menu item clicked");
		
		// TODO: Implement
		Log::CoreInfo("Set Current Scene As Start Scene not yet implemented");
	}

	void EditorLayer::UI_ShowNewScenePopup()
	{
		if (!s_ShowNewScenePopup)
			return;

		ImGui::OpenPopup("New Scene");

		if (ImGui::BeginPopupModal("New Scene", &s_ShowNewScenePopup, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Create a new scene");
			ImGui::Separator();

			ImGui::InputText("Scene Name", s_NewSceneNameBuffer, sizeof(s_NewSceneNameBuffer));
			ImGui::InputText("Location", s_NewSceneFolderBuffer, sizeof(s_NewSceneFolderBuffer));
			ImGui::SameLine();
			if (ImGui::Button("Browse..."))
			{
				std::filesystem::path assets_dir = Project::GetActiveAssetDirectory();
				std::string folder = FileDialog::OpenFolder();
				
				if (!folder.empty())
				{
					// Verify the selected folder is within the assets directory
					std::filesystem::path selected_path(folder);
				
					// Normalize paths for comparison
					std::string assets_str = std::filesystem::canonical(assets_dir).string();
					std::string selected_str = std::filesystem::canonical(selected_path).string();
				
					if (selected_str.find(assets_str) == 0 || selected_str == assets_str)
					{
						std::strncpy(s_NewSceneFolderBuffer, folder.c_str(), sizeof(s_NewSceneFolderBuffer) - 1);
						s_NewSceneFolderBuffer[sizeof(s_NewSceneFolderBuffer) - 1] = '\0';
					}
					else
					{
						Log::CoreError("Scene must be created within the project assets folder");
					}
				}
			}

			ImGui::Separator();

			if (ImGui::Button("Create", ImVec2(120, 0)))
			{
				// Validate scene name
				std::string scene_name(s_NewSceneNameBuffer);
				std::string scene_location(s_NewSceneFolderBuffer);
			
				if (scene_name.empty())
				{
					Log::CoreError("Scene name cannot be empty");
				}
				else if (scene_location.empty())
				{
					Log::CoreError("Scene location cannot be empty");
				}
				else
				{
					// Construct full scene path
					std::filesystem::path scene_path = std::filesystem::path(scene_location) / (scene_name + ".igscene");
				
					// Check if file already exists
					if (std::filesystem::exists(scene_path))
					{
						Log::CoreError("Scene file already exists: {}", scene_path.string());
					}
					else
					{
						// Create directory if it doesn't exist
						std::filesystem::path parent_dir = scene_path.parent_path();
						if (!std::filesystem::exists(parent_dir))
						{
							std::filesystem::create_directories(parent_dir);
						}
					
						// Create a new empty scene
						auto new_scene = std::make_shared<Scene>();
						new_scene->SetName(scene_name);
				
						// Serialize the scene to file
						SceneSerializer serializer;
						serializer.Serialize(*new_scene, scene_path);
				
						Log::CoreInfo("New scene created: {}", scene_path.string());
				
						// Register the new scene file as an asset
						AssetManager::ImportAsset(scene_path);
				
						// Load the newly created scene into the editor
						if (auto* app = dynamic_cast<EditorApp*>(&Application::Get()))
						{
							if (auto* scene_layer = app->GetSceneLayer())
							{
								scene_layer->LoadScene(scene_path);
							}
						}
				
						s_ShowNewScenePopup = false;
					}
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				s_ShowNewScenePopup = false;
			}
			
			ImGui::EndPopup();
		}
	}

	void EditorLayer::ProcessDeferredSceneLoad()
	{
		if (s_LoadSceneFilePathBuffer[0] != '\0')
		{
			std::string filepath(s_LoadSceneFilePathBuffer);
			Log::CoreInfo("Processing deferred scene load: {}", filepath);
			
			// Load the scene through EditorSceneLayer
			if (auto* app = dynamic_cast<EditorApp*>(&Application::Get()))
			{
				if (auto* scene_layer = app->GetSceneLayer())
				{
					scene_layer->LoadScene(filepath);
				}
				else
				{
					Log::CoreError("Failed to access EditorSceneLayer for scene loading");
				}
			}
			
			// Clear buffer
			s_LoadSceneFilePathBuffer[0] = '\0';
		}
	}

	void EditorLayer::ProcessDeferredSceneSaveAs()
	{
		if (s_SaveSceneAsFolderBuffer[0] != '\0')
		{
			std::string folder(s_SaveSceneAsFolderBuffer);
			Log::CoreInfo("Processing deferred scene save as: {}", folder);
			
			// TODO: Implement actual scene save as
			Log::CoreInfo("Scene save as not yet implemented");
			
			// Clear buffer
			s_SaveSceneAsFolderBuffer[0] = '\0';
		}
	}

} // namespace ignis