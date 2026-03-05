#include "Editor/Panels/ViewportPanel.h"
#include "Editor/EditorSceneLayer.h"
#include "Ignis/Project/Project.h"

namespace ignis {

	ViewportPanel::ViewportPanel(Renderer* renderer, EditorSceneLayer* editor_scene_layer)
		: m_renderer(renderer), m_editor_scene_layer(editor_scene_layer), m_viewport_size(0.0f, 0.0f)
	{
		m_imgui_texture_helper = ImGuiTextureHelper::Create();
	}

	void ViewportPanel::OnImGuiRender()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos;
		ImVec2 work_size = viewport->WorkSize;

		float hierarchy_width = 250.0f;
		float properties_width = 300.0f;
		float console_height = 200.0f;

		ImVec2 viewport_pos(work_pos.x + hierarchy_width, work_pos.y);
		ImVec2 viewport_size(work_size.x - hierarchy_width - properties_width,
		                     work_size.y - console_height);

		ImGui::SetNextWindowPos(viewport_pos);
		ImGui::SetNextWindowSize(viewport_size);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		if (ImGui::Begin("Viewport", nullptr, window_flags))
		{
			// Track focus state and calculate viewport bounds for camera input gating
			m_is_focused = ImGui::IsWindowFocused();
			ImVec2 window_pos = ImGui::GetWindowPos();
			ImVec2 window_size = ImGui::GetWindowSize();
			m_viewport_min_bound = window_pos;
			m_viewport_max_bound = ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y);
			
			// Render fixed toolbar at top
			RenderToolbar();
			
			// Get remaining content region for framebuffer (after toolbar)
			ImVec2 content_size = ImGui::GetContentRegionAvail();
			m_viewport_size = content_size;

			auto framebuffer = m_renderer->GetFramebuffer();
			if (framebuffer && content_size.x > 0 && content_size.y > 0)
			{
				// Resize framebuffer if viewport size changed
				uint32_t fb_width = framebuffer->GetWidth();
				uint32_t fb_height = framebuffer->GetHeight();
				
				if (fb_width != (uint32_t)content_size.x || fb_height != (uint32_t)content_size.y)
				{
					framebuffer->Resize((uint32_t)content_size.x, (uint32_t)content_size.y);
					m_renderer->SetViewport(0, 0, (uint32_t)content_size.x, (uint32_t)content_size.y);
				}
				
				auto color_texture = framebuffer->GetColorAttachment(0);
				if (color_texture)
				{
					m_imgui_texture_helper->RenderTexture(color_texture, content_size);
				}
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void ViewportPanel::OnEvent(EventBase& event)
	{
	}

	void ViewportPanel::RenderToolbar()
	{
		// Validation checks
		if (!m_editor_scene_layer)
			return;
		
		if (!Project::GetActive())
			return;
		
		auto current_scene = m_editor_scene_layer->GetScene();
		if (!current_scene)
			return;
		
		// Toolbar configuration
		const float toolbar_height = 32.0f;
		const float button_size = 24.0f;
		
		// Create fixed toolbar child window
		ImGui::BeginChild("##ViewportToolbar", ImVec2(0, toolbar_height), false, 
		                  ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		
		// Draw toolbar background
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 toolbar_min = ImGui::GetWindowPos();
		ImVec2 toolbar_max = ImVec2(toolbar_min.x + ImGui::GetWindowWidth(), 
		                             toolbar_min.y + toolbar_height);
		draw_list->AddRectFilled(toolbar_min, toolbar_max, 
		                         IM_COL32(30, 30, 30, 255));
		
		// Center buttons horizontally
		float content_width = ImGui::GetContentRegionAvail().x;
		float buttons_width = button_size * 2 + ImGui::GetStyle().ItemSpacing.x;
		ImGui::SetCursorPosX((content_width - buttons_width) * 0.5f);
		ImGui::SetCursorPosY((toolbar_height - button_size) * 0.5f);
		
		// Get current scene state
		auto scene_state = m_editor_scene_layer->GetSceneState();
		bool is_edit_mode = (scene_state == EditorSceneLayer::SceneState::Edit);
		
		// Play/Pause button with text label
		const char* button_label = is_edit_mode ? "P" : "S";
		ImVec4 button_color = is_edit_mode ? 
			ImVec4(0.3f, 0.8f, 0.3f, 1.0f) :  // Green for Play
			ImVec4(0.9f, 0.5f, 0.2f, 1.0f);   // Orange for Stop
		
		ImGui::PushStyleColor(ImGuiCol_Button, button_color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(
			button_color.x * 1.2f, 
			button_color.y * 1.2f, 
			button_color.z * 1.2f, 
			1.0f
		));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(
			button_color.x * 0.8f, 
			button_color.y * 0.8f, 
			button_color.z * 0.8f, 
			1.0f
		));
		
		if (ImGui::Button(button_label, ImVec2(button_size, button_size)))
		{
			if (is_edit_mode)
			{
				m_editor_scene_layer->OnScenePlay();
			}
				else
			{
				m_editor_scene_layer->OnSceneStop();
			}
		}
		
		ImGui::PopStyleColor(3);
		
		// Tooltip
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip(is_edit_mode ? 
				"Play (Run Scripts)" : 
				"Stop (Return to Editor)");
		}
		
		ImGui::EndChild();
	}

} // namespace ignis
