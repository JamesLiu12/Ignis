#include "PropertiesPanel.h"
#include <imgui.h>

namespace ignis {

	void PropertiesPanel::OnImGuiRender()
	{
		// Fixed position at right side of screen
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos;
		ImVec2 work_size = viewport->WorkSize;

		float panel_width = 300.0f;
		
		// Position: right side, full height
		ImGui::SetNextWindowPos(ImVec2(work_pos.x + work_size.x - panel_width, work_pos.y));
		ImGui::SetNextWindowSize(ImVec2(panel_width, work_size.y));

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

		if (ImGui::Begin("Properties", nullptr, window_flags))
		{
			if (m_selected_entity)
			{
				// Render entity name
				if (m_selected_entity->HasComponent<TagComponent>())
				{
					auto& tag = m_selected_entity->GetComponent<TagComponent>();
					ImGui::Text("Entity: %s", tag.Tag.c_str());
					ImGui::Separator();
				}
				
				// Render Transform Component
				if (m_selected_entity->HasComponent<TransformComponent>())
				{
					auto& transform = m_selected_entity->GetComponent<TransformComponent>();
					RenderTransformComponent(transform);
				}
				
				// Render Light Component
				if (m_selected_entity->HasComponent<LightComponent>())
				{
					auto& light = m_selected_entity->GetComponent<LightComponent>();
					RenderLightComponent(light);
				}
			}
			else
			{
				ImGui::TextDisabled("No entity selected");
			}
		}
		ImGui::End();
	}
	
	void PropertiesPanel::RenderTransformComponent(TransformComponent& transform)
	{
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::DragFloat3("Position", &transform.Translation.x, 0.1f);
			ImGui::DragFloat3("Rotation", &transform.Rotation.x, 0.1f);
			ImGui::DragFloat3("Scale", &transform.Scale.x, 0.1f);
			ImGui::Spacing();
		}
	}
	
	void PropertiesPanel::RenderLightComponent(LightComponent& light)
	{
		if (ImGui::CollapsingHeader("Light", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// Light Type Dropdown
			const char* light_types[] = { "Directional", "Point", "Spot" };
			int current_type = static_cast<int>(light.LightType);
			if (ImGui::Combo("Type", &current_type, light_types, 3))
			{
				light.LightType = static_cast<LightComponent::Type>(current_type);
			}
			
			// Color Picker
			ImGui::ColorEdit3("Color", &light.Color.x);
			
			// Intensity Slider
			ImGui::SliderFloat("Intensity", &light.Intensity, 0.0f, 10.0f);
			
			// Type-specific properties
			if (light.LightType == LightComponent::Type::Directional)
			{
				ImGui::Separator();
				ImGui::Text("Directional Light Settings");
				ImGui::DragFloat3("Direction", &light.Direction.x, 0.01f, -1.0f, 1.0f);
			}
			else if (light.LightType == LightComponent::Type::Point)
			{
				ImGui::Separator();
				ImGui::Text("Point Light Settings");
				ImGui::SliderFloat("Range", &light.Range, 0.1f, 100.0f);
				ImGui::SliderFloat("Attenuation", &light.Attenuation, 0.0f, 2.0f);
			}
			else if (light.LightType == LightComponent::Type::Spot)
			{
				ImGui::Separator();
				ImGui::Text("Spot Light Settings");
				ImGui::DragFloat3("Direction", &light.Direction.x, 0.01f, -1.0f, 1.0f);
				ImGui::SliderFloat("Range", &light.Range, 0.1f, 100.0f);
				ImGui::SliderFloat("Attenuation", &light.Attenuation, 0.0f, 2.0f);
				ImGui::SliderFloat("Inner Cone Angle", &light.InnerConeAngle, 0.0f, 90.0f);
				ImGui::SliderFloat("Outer Cone Angle", &light.OuterConeAngle, 0.0f, 90.0f);
			}
			
			ImGui::Spacing();
		}
	}

} // namespace ignis