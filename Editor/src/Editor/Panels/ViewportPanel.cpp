#include "Editor/Panels/ViewportPanel.h"

namespace ignis {

	ViewportPanel::ViewportPanel(Renderer* renderer)
		: m_renderer(renderer), m_viewport_size(0.0f, 0.0f)
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

} // namespace ignis
