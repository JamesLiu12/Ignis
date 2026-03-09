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

		// Center button
		const float button_width = 100.0f;
		const float button_height = 35.0f;
		
		ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - button_width) * 0.5f);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);

		// Single toggle button - changes label and color based on state
		const char* button_label = is_edit_mode ? "Play" : "Stop";
		const char* tooltip_text = is_edit_mode ? "Enter Play Mode (Run Scripts)" : "Return to Edit Mode";
		
		ImVec4 button_color = is_edit_mode ? 
			ImVec4(0.3f, 0.8f, 0.3f, 1.0f) :  // Green for Play
			ImVec4(0.9f, 0.5f, 0.2f, 1.0f);   // Orange for Stop
		
		ImVec4 button_hover = is_edit_mode ?
			ImVec4(0.4f, 0.9f, 0.4f, 1.0f) :
			ImVec4(1.0f, 0.6f, 0.3f, 1.0f);
		
		ImVec4 button_active = is_edit_mode ?
			ImVec4(0.2f, 0.7f, 0.2f, 1.0f) :
			ImVec4(0.8f, 0.4f, 0.1f, 1.0f);
		
		ImGui::PushStyleColor(ImGuiCol_Button, button_color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, button_hover);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, button_active);
		
		if (ImGui::Button(button_label, ImVec2(button_width, button_height)))
		{
			if (is_edit_mode)
				m_editor_scene_layer->OnScenePlay();
			else
				m_editor_scene_layer->OnSceneStop();
		}
		
		ImGui::PopStyleColor(3);
		
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", tooltip_text);
	}

	void ControlPanel::OnEvent(EventBase& event)
	{
		// Future: Handle panel-specific events
	}

} // namespace ignis
