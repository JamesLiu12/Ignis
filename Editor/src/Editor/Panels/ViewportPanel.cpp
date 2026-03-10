#include "Editor/Panels/ViewportPanel.h"
#include "Editor/EditorSceneLayer.h"
#include "Ignis/Project/Project.h"

#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include <ImGuizmo.h>

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

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		if (ImGui::Begin("Viewport", nullptr, window_flags))
		{
			// Track focus state and calculate viewport bounds for camera input gating
			m_is_focused = ImGui::IsWindowFocused();
			ImVec2 window_pos = ImGui::GetWindowPos();
			ImVec2 window_size = ImGui::GetWindowSize();
			m_viewport_min_bound = window_pos;
			m_viewport_max_bound = ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y);
			
			// Get content region for framebuffer
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

			if (m_editor_scene_layer &&
				m_editor_scene_layer->GetSceneState() == EditorSceneLayer::SceneState::Edit)
			{
				Entity    selected = m_editor_scene_layer->GetSelectedEntity();
				GizmoMode mode = m_editor_scene_layer->GetGizmoMode();

				if (selected && selected.IsValid() && mode != GizmoMode::None)
				{
					ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
					switch (mode)
					{
					case GizmoMode::Translate: operation = ImGuizmo::TRANSLATE; break;
					case GizmoMode::Rotate:    operation = ImGuizmo::ROTATE;    break;
					case GizmoMode::Scale:     operation = ImGuizmo::SCALE;     break;
					default: break;
					}

					ImGuizmo::SetOrthographic(false);
					ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
					ImGuizmo::SetRect(m_viewport_min_bound.x, m_viewport_min_bound.y,
						m_viewport_size.x, m_viewport_size.y);

					auto      camera = m_editor_scene_layer->GetEditorCamera();
					glm::mat4 view = camera->GetView();
					glm::mat4 proj = camera->GetProjection();

					glm::mat4 world_transform = selected.GetWorldTransform();

					bool manipulated = ImGuizmo::Manipulate(
						glm::value_ptr(view),
						glm::value_ptr(proj),
						operation,
						ImGuizmo::LOCAL,
						glm::value_ptr(world_transform));

					if (manipulated)
					{
						glm::mat4 local_transform = world_transform;
						Entity parent = selected.GetParent();
						if (parent && parent.IsValid())
						{
							local_transform =
								glm::inverse(parent.GetWorldTransform()) * world_transform;
						}

						glm::vec3 translation, skew, scale;
						glm::vec4 perspective;
						glm::quat rotation_quat;
						glm::decompose(local_transform,
							scale, rotation_quat,
							translation, skew, perspective);

						auto& tc = selected.GetComponent<TransformComponent>();

						switch (mode)
						{
						case GizmoMode::Translate:
							tc.Translation = translation;
							break;

						case GizmoMode::Rotate:
							tc.Rotation = glm::eulerAngles(glm::normalize(rotation_quat));
							break;

						case GizmoMode::Scale:
							tc.Scale = scale;
							break;

						default: break;
						}
					}
				}
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}

	void ViewportPanel::OnEvent(EventBase& event)
	{
	}

	bool ViewportPanel::IsPointInViewport(float x, float y) const
	{
		return x >= m_viewport_min_bound.x && x <= m_viewport_max_bound.x
			&& y >= m_viewport_min_bound.y && y <= m_viewport_max_bound.y;
	}

} // namespace ignis
