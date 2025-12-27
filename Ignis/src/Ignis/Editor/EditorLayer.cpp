#include "EditorLayer.h"
#include "Ignis/Core/Log.h"
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
		Log::CoreInfo("EditorLayer detached");
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