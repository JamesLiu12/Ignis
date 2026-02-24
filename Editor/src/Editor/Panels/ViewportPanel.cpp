#include "Editor/Panels/ViewportPanel.h"

namespace ignis {

	ViewportPanel::ViewportPanel(Renderer* renderer)
		: m_renderer(renderer), m_viewport_size(0.0f, 0.0f)
	{
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
			ImVec2 content_size = ImGui::GetContentRegionAvail();
			m_viewport_size = content_size;

			auto framebuffer = m_renderer->GetFramebuffer();
			if (framebuffer && content_size.x > 0 && content_size.y > 0)
			{
				auto color_texture = framebuffer->GetColorAttachment(0);
				if (color_texture)
				{
					ImGui::Image(
						(void*)(intptr_t)color_texture->GetRendererID(),
						content_size,
						ImVec2(0, 1),
						ImVec2(1, 0)
					);
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
