#include "Editor/EditorLayer.h"
#include <imgui.h>
#include "Ignis/Core/File/FileDialog.h"
#include "Editor/Core/ProjectManager.h"

namespace ignis {

	// Initialize static buffers
	char EditorLayer::s_OpenProjectFilePathBuffer[512] = "";
	char EditorLayer::s_SaveProjectAsFolderBuffer[512] = "";
	char EditorLayer::s_NewProjectFolderBuffer[512] = "";
	char EditorLayer::s_NewProjectNameBuffer[128] = "";
	bool EditorLayer::s_ShowNewProjectPopup = false;

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
		// Render menu bar
		RenderMenuBar();

		// Render all panels managed by PanelManager
		m_panel_manager->OnImGuiRender();

		// Handle new project popup
		if (s_ShowNewProjectPopup)
		{
			UI_ShowNewProjectPopup();
		}

		// Process deferred operations (safe after ImGui rendering)
		ProcessDeferredProjectLoad();
		ProcessDeferredSaveAs();
	}

	void EditorLayer::OnEvent(EventBase& event)
	{
		// Forward events to all panels
		m_panel_manager->OnEvent(event);
	}

	void EditorLayer::OpenProject()
	{
		std::string folder = FileDialog::OpenFolder();
		if (folder.empty())
			return;

		// Find project file
		auto projectFile = ProjectManager::FindProjectFile(folder);
		if (!projectFile)
			return;

		// Defer actual loading
		strcpy(s_OpenProjectFilePathBuffer, projectFile->string().c_str());
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

	void EditorLayer::UI_ShowNewProjectPopup()
	{
		// TODO: Phase 3 - Implement new project creation popup
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
				// TODO: Phase 3 - Actually create the project
				Log::CoreInfo("Create project: {} at {}", s_NewProjectNameBuffer, s_NewProjectFolderBuffer);
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
				if (ImGui::MenuItem("New Project..."))
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

				if (ImGui::MenuItem("Load Project...", openShortcut))
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
				if (ImGui::MenuItem("Save Project As...", saveAsShortcut, false, hasProject))
				{
					SaveProjectAs();
				}

				if (ImGui::MenuItem("Close Project", nullptr, false, hasProject))
				{
					CloseProject();
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
} // namespace ignis