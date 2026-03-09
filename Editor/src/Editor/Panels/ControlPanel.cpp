#include "Editor/Panels/ControlPanel.h"
#include "Editor/EditorSceneLayer.h"
#include "Ignis/Project/Project.h"

#include <imgui.h>

namespace ignis {

	ControlPanel::ControlPanel(EditorSceneLayer* editor_scene_layer)
		: m_editor_scene_layer(editor_scene_layer)
	{
	}

	void ControlPanel::OnImGuiRender()
	{
		// Calculate position and size
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos;
		ImVec2 work_size = viewport->WorkSize;

		float hierarchy_width = 250.0f;
		float properties_width = 300.0f;
		float console_height = 200.0f;

		// Bottom-right corner
		ImVec2 panel_pos(work_pos.x + work_size.x - properties_width,
		                 work_pos.y + work_size.y - console_height);
		ImVec2 panel_size(properties_width, console_height);

		ImGui::SetNextWindowPos(panel_pos);
		ImGui::SetNextWindowSize(panel_size);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | 
		                                ImGuiWindowFlags_NoResize | 
		                                ImGuiWindowFlags_NoCollapse;

		if (ImGui::Begin("Control Panel", nullptr, window_flags))
		{
			RenderPlayStopButtons();
			
			// Future: Add more controls here
			// - Scene settings
			// - Editor preferences toggle
			// - Build/Export buttons
		}
		ImGui::End();
	}

	void ControlPanel::RenderPlayStopButtons()
	{
		if (!m_editor_scene_layer || !Project::GetActive())
			return;

		auto current_scene = m_editor_scene_layer->GetScene();
		if (!current_scene)
			return;

		auto scene_state = m_editor_scene_layer->GetSceneState();
		bool is_edit_mode = (scene_state == EditorSceneLayer::SceneState::Edit);

		// Center buttons
		const float button_width = 80.0f;
		const float button_height = 30.0f;
		const float spacing = 10.0f;
		const float total_width = button_width * 2 + spacing;
		
		ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - total_width) * 0.5f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);

		// Play button
		if (is_edit_mode)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.9f, 0.4f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
			
			if (ImGui::Button("Play", ImVec2(button_width, button_height)))
			{
				m_editor_scene_layer->OnScenePlay();
			}
			
			ImGui::PopStyleColor(3);
			
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Enter Play Mode (Run Scripts)");
		}
		else
		{
			ImGui::BeginDisabled();
			ImGui::Button("Play", ImVec2(button_width, button_height));
			ImGui::EndDisabled();
		}

		ImGui::SameLine(0.0f, spacing);

		// Stop button
		if (!is_edit_mode)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.5f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.6f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.4f, 0.1f, 1.0f));
			
			if (ImGui::Button("Stop", ImVec2(button_width, button_height)))
			{
				m_editor_scene_layer->OnSceneStop();
			}
			
			ImGui::PopStyleColor(3);
			
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Return to Edit Mode");
		}
		else
		{
			ImGui::BeginDisabled();
			ImGui::Button("Stop", ImVec2(button_width, button_height));
			ImGui::EndDisabled();
		}
	}

	void ControlPanel::OnEvent(EventBase& event)
	{
		// Future: Handle panel-specific events
	}

} // namespace ignis
