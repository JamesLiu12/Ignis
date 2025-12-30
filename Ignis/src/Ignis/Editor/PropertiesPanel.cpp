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
			if (auto entity = m_selected_entity.lock())
			{
				// Render entity name
				if (entity->HasComponent<TagComponent>())
				{
					auto& tag = entity->GetComponent<TagComponent>();
					ImGui::Text("Entity: %s", tag.Tag.c_str());
					ImGui::Separator();
				}
				
				// Render Transform Component
				if (entity->HasComponent<TransformComponent>())
				{
					auto& transform = entity->GetComponent<TransformComponent>();
					RenderTransformComponent(transform);
				}
				
				// Render Directional Light Component
				if (entity->HasComponent<DirectionalLightComponent>())
				{
					auto& light = entity->GetComponent<DirectionalLightComponent>();
					RenderDirectionalLightComponent(light);
				}
				
				// Render Point Light Component
				if (entity->HasComponent<PointLightComponent>())
				{
					auto& light = entity->GetComponent<PointLightComponent>();
					RenderPointLightComponent(light);
				}
				
				// Render Spot Light Component
				if (entity->HasComponent<SpotLightComponent>())
				{
					auto& light = entity->GetComponent<SpotLightComponent>();
					RenderSpotLightComponent(light);
				}
				
				// Add Component Section
				ImGui::Separator();
				ImGui::Text("Add Component");
				
				if (!entity->HasComponent<DirectionalLightComponent>())
				{
					if (ImGui::Button("Add Directional Light"))
					{
						entity->AddComponent<DirectionalLightComponent>();
					}
				}
				
				if (!entity->HasComponent<PointLightComponent>())
				{
					if (ImGui::Button("Add Point Light"))
					{
						entity->AddComponent<PointLightComponent>();
					}
				}
				
				if (!entity->HasComponent<SpotLightComponent>())
				{
					if (ImGui::Button("Add Spot Light"))
					{
						entity->AddComponent<SpotLightComponent>();
					}
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
	
	void PropertiesPanel::RenderDirectionalLightComponent(DirectionalLightComponent& light)
	{
		ImGui::PushID("DirectionalLight");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Directional Light", flags);
		
		// Remove button on the same line as header
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		bool remove = ImGui::Button("X");
		
		if (open)
		{
			ImGui::ColorEdit3("Color", &light.Color.x);
			ImGui::SliderFloat("Intensity", &light.Intensity, 0.0f, 10.0f);
			ImGui::DragFloat3("Direction", &light.Direction.x, 0.01f, -1.0f, 1.0f);
			ImGui::Spacing();
		}
		
		if (remove)
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<DirectionalLightComponent>();
			}
		}
		
		ImGui::PopID();
	}
	
	void PropertiesPanel::RenderPointLightComponent(PointLightComponent& light)
	{
		ImGui::PushID("PointLight");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Point Light", flags);
		
		// Remove button on the same line as header
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		bool remove = ImGui::Button("X");
		
		if (open)
		{
			ImGui::ColorEdit3("Color", &light.Color.x);
			ImGui::SliderFloat("Intensity", &light.Intensity, 0.0f, 10.0f);
			ImGui::SliderFloat("Range", &light.Range, 0.1f, 100.0f);
			ImGui::SliderFloat("Attenuation", &light.Attenuation, 0.0f, 2.0f);
			ImGui::Spacing();
		}
		
		if (remove)
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<PointLightComponent>();
			}
		}
		
		ImGui::PopID();
	}
	
	void PropertiesPanel::RenderSpotLightComponent(SpotLightComponent& light)
	{
		ImGui::PushID("SpotLight");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Spot Light", flags);
		
		// Remove button on the same line as header
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		bool remove = ImGui::Button("X");
		
		if (open)
		{
			ImGui::ColorEdit3("Color", &light.Color.x);
			ImGui::SliderFloat("Intensity", &light.Intensity, 0.0f, 10.0f);
			ImGui::DragFloat3("Direction", &light.Direction.x, 0.01f, -1.0f, 1.0f);
			ImGui::SliderFloat("Range", &light.Range, 0.1f, 100.0f);
			ImGui::SliderFloat("Attenuation", &light.Attenuation, 0.0f, 2.0f);
			ImGui::SliderFloat("Inner Cone Angle", &light.InnerConeAngle, 0.0f, 90.0f);
			ImGui::SliderFloat("Outer Cone Angle", &light.OuterConeAngle, 0.0f, 90.0f);
			ImGui::Spacing();
		}
		
		if (remove)
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<SpotLightComponent>();
			}
		}
		
		ImGui::PopID();
	}

} // namespace ignis