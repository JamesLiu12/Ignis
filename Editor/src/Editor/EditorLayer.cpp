#include "Editor/EditorLayer.h"
#include <imgui.h>

namespace ignis {

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
		
		// Put your test project here
		OpenProject("MyProject/MyProject.igproj");
	}

	void EditorLayer::OnDetach()
	{
		// Put your test project here
		SaveProject("MyProject/MyProjectSaved.igproj");
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
	}

	void EditorLayer::OnEvent(EventBase& event)
	{
		// Forward events to all panels
		m_panel_manager->OnEvent(event);
	}

	void EditorLayer::OpenProject(const std::filesystem::path& filepath)
	{
		ProjectSerializer project_serializer;

		if (auto project = project_serializer.Deserialize(filepath))
		{
			Project::SetActive(project);
		}
		else
		{
			Log::CoreError("Failed to open project: {}", filepath.string());
		}
	}
	
	void EditorLayer::SaveProject(const std::filesystem::path& filepath)
	{
		ProjectSerializer project_serializer;

		if (auto project = Project::GetActive())
		{
			if (!project_serializer.Serialize(*project, filepath))
			{
				Log::CoreError("Failed to save project: {}", filepath.string());
			}
		}
		else
		{
			Log::CoreError("No active project to save");
		}
	}

	void EditorLayer::RenderMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
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