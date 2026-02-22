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
	}

	void EditorLayer::OnDetach()
	{
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

	void EditorLayer::RenderMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("View"))
			{
				auto& panels = m_panel_manager->GetPanels();
				
				// Core panels that should not be toggleable (always visible)
				static const std::unordered_set<std::string> core_panels = {
					"Console", "Properties", "SceneHierarchy"
				};
				
				for (auto& panel_data : panels)
				{
					// Skip core panels - they're always visible
					if (core_panels.find(panel_data.ID) != core_panels.end())
						continue;
						
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