#include "PropertiesPanel.h"
#include "Ignis/Asset/AssetManager.h"
#include "Ignis/Renderer/Renderer.h"
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
			// Section 1: Current Mesh Editor
			if (m_current_mesh)
			{
				RenderMeshEditor();
				ImGui::Separator();
			}
			
			// Section 2: Selected Entity
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
			else if (!m_current_mesh)
			{
				ImGui::TextDisabled("No entity or mesh selected");
			}
		}
		ImGui::End();
	}
	
	void PropertiesPanel::RenderMeshEditor()
	{
		if (ImGui::CollapsingHeader("Current Mesh", ImGuiTreeNodeFlags_DefaultOpen))
		{
			// Display mesh path from metadata
			std::string mesh_path = "Unknown";
			if (auto* metadata = AssetManager::GetMetadata(m_current_mesh->GetHandle()))
			{
				mesh_path = metadata->FilePath.string();
			}
			
			ImGui::Text("Mesh: %s", mesh_path.c_str());
			ImGui::Text("Materials: %zu", m_current_mesh->GetMaterialsData().size());
			ImGui::Separator();
			
			// Mesh Transform
			if (m_mesh_transform && ImGui::TreeNodeEx("Mesh Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::DragFloat3("Translation", &m_mesh_transform->Translation.x, 0.1f);
				ImGui::DragFloat3("Rotation", &m_mesh_transform->Rotation.x, 0.1f);
				ImGui::DragFloat3("Scale", &m_mesh_transform->Scale.x, 0.1f);
				ImGui::TreePop();
			}
			
			ImGui::Separator();
			
			// Materials
			RenderMaterialsUI(m_current_mesh);
		}
	}
	
	void PropertiesPanel::RenderMaterialsUI(std::shared_ptr<Mesh> mesh)
	{
		const auto& materials = mesh->GetMaterialsData();
		
		for (uint32_t i = 0; i < materials.size(); ++i)
		{
			ImGui::PushID(i);
			
			std::string header = "Material " + std::to_string(i);
			if (ImGui::TreeNodeEx(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
			{
				RenderTextureSlot(mesh, i, MaterialType::Albedo, "Albedo Map");
				RenderTextureSlot(mesh, i, MaterialType::Normal, "Normal Map");
				RenderTextureSlot(mesh, i, MaterialType::Metal, "Metallic Map");
				RenderTextureSlot(mesh, i, MaterialType::Roughness, "Roughness Map");
				RenderTextureSlot(mesh, i, MaterialType::Emissive, "Emissive Map");
				RenderTextureSlot(mesh, i, MaterialType::AO, "AO Map");
				ImGui::TreePop();
			}
			
			ImGui::PopID();
		}
	}
	
	void PropertiesPanel::RenderTextureSlot(std::shared_ptr<Mesh> mesh, uint32_t material_index,
	                                        MaterialType type, const char* label)
	{
		ImGui::PushID(label);
		
		const auto& materials = mesh->GetMaterialsData();
		if (material_index >= materials.size())
		{
			ImGui::PopID();
			return;
		}
		
		const MaterialData& mat_data = materials[material_index];
		
		// Get current texture
		AssetHandle current_handle = AssetHandle::InvalidUUID;
		switch (type)
		{
			case MaterialType::Albedo:    current_handle = mat_data.AlbedoMap; break;
			case MaterialType::Normal:    current_handle = mat_data.NormalMap; break;
			case MaterialType::Metal:     current_handle = mat_data.MetalnessMap; break;
			case MaterialType::Roughness: current_handle = mat_data.RoughnessMap; break;
			case MaterialType::Emissive:  current_handle = mat_data.EmissiveMap; break;
			case MaterialType::AO:        current_handle = mat_data.AOMap; break;
		}
		
		// Display current texture
		std::string preview = "None";
		if (current_handle.IsValid())
		{
			// Check if it's a memory-only asset (default texture)
			if (AssetManager::IsMemoryAsset(current_handle))
			{
				// Identify which default texture it is
				if (current_handle == Renderer::GetWhiteTextureHandle())
					preview = "White Texture";
				else if (current_handle == Renderer::GetBlackTextureHandle())
					preview = "Black Texture";
				else if (current_handle == Renderer::GetDefaultNormalTextureHandle())
					preview = "Default Normal";
				else if (current_handle == Renderer::GetDefaultRoughnessTextureHandle())
					preview = "Default Roughness";
				else
					preview = "Memory Asset";
			}
			else if (auto* metadata = AssetManager::GetMetadata(current_handle))
			{
				// File-based texture
				preview = metadata->FilePath.string();
				// Show just filename
				size_t last_slash = preview.find_last_of("/\\");
				if (last_slash != std::string::npos)
					preview = preview.substr(last_slash + 1);
			}
		}
		
		ImGui::Text("%s:", label);
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", preview.c_str());
		
		// Dropdown for default textures (use unique ID per slot)
		const char* options[] = {
			"Keep Current",
			"None (Clear)",
			"White Texture",
			"Black Texture",
			"Default Normal",
			"Default Roughness"
		};
		
		int selected = 0; // Always start at "Keep Current"
		if (ImGui::Combo("##TextureOptions", &selected, options, IM_ARRAYSIZE(options)))
		{
			switch (selected)
			{
				case 0: // Keep Current
					break;
				case 1: // None
					mesh->SetMaterialDataTexture(material_index, type, AssetHandle::InvalidUUID);
					break;
				case 2: // White Texture
					mesh->SetMaterialDataTexture(material_index, type, Renderer::GetWhiteTextureHandle());
					break;
				case 3: // Black Texture
					mesh->SetMaterialDataTexture(material_index, type, Renderer::GetBlackTextureHandle());
					break;
				case 4: // Default Normal
					mesh->SetMaterialDataTexture(material_index, type, Renderer::GetDefaultNormalTextureHandle());
					break;
				case 5: // Default Roughness
					mesh->SetMaterialDataTexture(material_index, type, Renderer::GetDefaultRoughnessTextureHandle());
					break;
			}
		}
		
		ImGui::PopID();
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
		
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		bool remove = ImGui::Button("X");
		
		if (open)
		{
			ImGui::ColorEdit3("Color", &light.Color.x);
			ImGui::SliderFloat("Intensity", &light.Intensity, 0.0f, 10.0f);
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
		
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		bool remove = ImGui::Button("X");
		
		if (open)
		{
			ImGui::ColorEdit3("Color", &light.Color.x);
			ImGui::SliderFloat("Intensity", &light.Intensity, 0.0f, 10.0f);
			ImGui::SliderFloat("Range", &light.Range, 0.1f, 100.0f);
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
		
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		bool remove = ImGui::Button("X");
		
		if (open)
		{
			ImGui::ColorEdit3("Color", &light.Color.x);
			ImGui::SliderFloat("Intensity", &light.Intensity, 0.0f, 10.0f);
			ImGui::SliderFloat("Range", &light.Range, 0.1f, 100.0f);
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