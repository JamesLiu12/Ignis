#include "Editor/Panels/PropertiesPanel.h"
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
			// Section 1: ALWAYS show mesh/texture UI at top (if mesh exists)
			if (m_current_mesh_ptr && *m_current_mesh_ptr)
			{
				RenderMeshEditor();
				ImGui::Separator();
			}
			
			// Section 2: Show selected light entity controls below
			if (auto entity = m_selected_entity.lock())
			{
				// Render entity name
				if (entity->HasComponent<TagComponent>())
				{
					auto& tag = entity->GetComponent<TagComponent>();
					ImGui::Text("Light Entity: %s", tag.Tag.c_str());
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
				
				// Render Sky Light Component
				if (entity->HasComponent<SkyLightComponent>())
				{
					auto& light = entity->GetComponent<SkyLightComponent>();
					RenderSkyLightComponent(light);
				}
			}
			else if (!m_current_mesh_ptr || !(*m_current_mesh_ptr))
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
			if (auto* metadata = AssetManager::GetMetadata((*m_current_mesh_ptr)->GetHandle()))
			{
				mesh_path = metadata->FilePath.string();
			}
			
			ImGui::Text("Mesh: %s", mesh_path.c_str());
			ImGui::Text("Materials: %zu", (*m_current_mesh_ptr)->GetMaterialsData().size());
		
			// Replace Model button
			if (ImGui::Button("Replace Model...", ImVec2(-1, 0)))
			{
				std::string filepath = FileDialog::OpenFile();
				if (!filepath.empty())
				{
					LoadNewModel(filepath);
				}
			}
		
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
			RenderMaterialsUI(*m_current_mesh_ptr);
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
		AssetHandle current_handle = AssetHandle::Invalid;
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
			"Select",
			"Browse Texture...",
			"None (Clear)",
			"White Texture",
			"Black Texture",
			"Default Normal",
			"Default Roughness"
		};
		
		int selected = 0; // Always start at Select
		if (ImGui::Combo("##TextureOptions", &selected, options, IM_ARRAYSIZE(options)))
		{
			switch (selected)
			{
				case 0: // Select
					break;
				case 1: // Browse Texture
					{
						std::string filepath = FileDialog::OpenFile();
						if (!filepath.empty())
						{
							// Validate texture file extension
							std::string extension = filepath.substr(filepath.find_last_of(".") + 1);
							std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
							
							if (extension == "png" || extension == "jpg" || extension == "jpeg" || 
							    extension == "tga" || extension == "bmp" || extension == "hdr")
							{
								// Import texture
								AssetHandle texture_handle = AssetManager::ImportAsset(filepath);
								if (texture_handle.IsValid())
								{
									mesh->SetMaterialDataTexture(material_index, type, texture_handle);
									Log::Info("Imported texture: {}", filepath);
								}
								else
								{
									Log::Error("Failed to import texture: {}", filepath);
								}
							}
							else
							{
								Log::Error("Invalid texture format: {}. Supported: png, jpg, jpeg, tga, bmp, hdr", extension);
							}
						}
					}
					break;
				case 2: // None
					mesh->SetMaterialDataTexture(material_index, type, AssetHandle::Invalid);
					break;
				case 3: // White Texture
					mesh->SetMaterialDataTexture(material_index, type, Renderer::GetWhiteTextureHandle());
					break;
				case 4: // Black Texture
					mesh->SetMaterialDataTexture(material_index, type, Renderer::GetBlackTextureHandle());
					break;
				case 5: // Default Normal
					mesh->SetMaterialDataTexture(material_index, type, Renderer::GetDefaultNormalTextureHandle());
					break;
				case 6: // Default Roughness
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
	
	void PropertiesPanel::RenderSkyLightComponent(SkyLightComponent& light)
	{
		ImGui::PushID("SkyLight");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Sky Light", flags);
		
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		bool remove = ImGui::Button("X");
		
		if (open)
		{
			ImGui::SliderFloat("Intensity", &light.Intensity, 0.0f, 10.0f);
			ImGui::ColorEdit3("Tint", &light.Tint.x);
			ImGui::SliderFloat("Rotation", &light.Rotation, 0.0f, 360.0f);
			ImGui::SliderFloat("Skybox LOD", &light.SkyboxLod, 0.0f, 10.0f);
			ImGui::Spacing();
		}
		
		if (remove)
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<SkyLightComponent>();
			}
		}
		
		ImGui::PopID();
	}

	void PropertiesPanel::LoadNewModel(const std::string& filepath)
	{
		Log::Info("Attempting to load model: {}", filepath);
		
		// Validate file extension
		std::string extension = filepath.substr(filepath.find_last_of(".") + 1);
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
		
		if (extension != "obj" && extension != "fbx" && extension != "gltf" && extension != "glb")
		{
			Log::Error("Invalid model file format: {}. Supported formats: .obj, .fbx, .gltf, .glb", extension);
			return;
		}
		
		// Import mesh asset
		AssetHandle mesh_handle = AssetManager::ImportAsset(filepath);
		if (!mesh_handle.IsValid())
		{
			Log::Error("Failed to import model: {}", filepath);
			return;
		}
		
		// Load mesh
		auto new_mesh = AssetManager::GetAsset<Mesh>(mesh_handle);
		if (!new_mesh)
		{
			Log::Error("Failed to load mesh asset: {}", filepath);
			return;
		}
		
		// Validate mesh has vertices
		if (new_mesh->GetVertices().empty())
		{
			Log::Error("Loaded mesh has no vertices: {}", filepath);
			return;
		}
		
		// Replace current mesh
		if (m_current_mesh_ptr)
		{
			*m_current_mesh_ptr = new_mesh;
		}
		
		// Auto-scale model to reasonable size based on bounding box
		if (m_mesh_transform)
		{
			// Calculate bounding box from vertices
			const auto& vertices = new_mesh->GetVertices();
			if (!vertices.empty())
			{
				glm::vec3 min_bounds = vertices[0].Position;
				glm::vec3 max_bounds = vertices[0].Position;
				
				for (const auto& vertex : vertices)
				{
					min_bounds = glm::min(min_bounds, vertex.Position);
					max_bounds = glm::max(max_bounds, vertex.Position);
				}
				
				glm::vec3 size = max_bounds - min_bounds;
				float max_dimension = glm::max(glm::max(size.x, size.y), size.z);
				
				// Target size: models should fit within a 2-unit cube
				float target_size = 2.0f;
				float scale_factor = target_size / max_dimension;
				
				// Reset transform and apply auto-scale
				m_mesh_transform->Translation = glm::vec3(0.0f);
				m_mesh_transform->Rotation = glm::vec3(0.0f);
				m_mesh_transform->Scale = glm::vec3(scale_factor);
				
				Log::Info("Auto-scaled model: max dimension {:.3f} -> scale factor {:.3f}", max_dimension, scale_factor);
			}
		}
		
		// Initialize materials with default textures for missing slots
		const auto& materials = new_mesh->GetMaterialsData();
		Log::Info("Model loaded with {} materials", materials.size());
		
		for (uint32_t i = 0; i < materials.size(); ++i)
		{
			const MaterialData& mat = materials[i];
			
			// Only set defaults if texture is missing (not auto-imported)
			if (!mat.AlbedoMap.IsValid())
			{
				new_mesh->SetMaterialDataTexture(i, MaterialType::Albedo, 
				                                      Renderer::GetWhiteTextureHandle());
				Log::Info("Material {}: Applied default white texture to Albedo", i);
			}
			else
			{
				Log::Info("Material {}: Albedo texture auto-imported from model", i);
			}
			
			if (!mat.NormalMap.IsValid())
			{
				new_mesh->SetMaterialDataTexture(i, MaterialType::Normal, 
				                                      Renderer::GetDefaultNormalTextureHandle());
				Log::Info("Material {}: Applied default normal map", i);
			}
			else
			{
				Log::Info("Material {}: Normal map auto-imported from model", i);
			}
			
			if (!mat.MetalnessMap.IsValid())
			{
				new_mesh->SetMaterialDataTexture(i, MaterialType::Metal, 
				                                      Renderer::GetBlackTextureHandle());
			}
			
			if (!mat.RoughnessMap.IsValid())
			{
				new_mesh->SetMaterialDataTexture(i, MaterialType::Roughness, 
				                                      Renderer::GetDefaultRoughnessTextureHandle());
			}
			
			if (!mat.EmissiveMap.IsValid())
			{
				new_mesh->SetMaterialDataTexture(i, MaterialType::Emissive, 
				                                      Renderer::GetBlackTextureHandle());
			}
			
			if (!mat.AOMap.IsValid())
			{
				new_mesh->SetMaterialDataTexture(i, MaterialType::AO, 
				                                      Renderer::GetWhiteTextureHandle());
			}
		}
		
		Log::Info("Successfully loaded model: {}", filepath);
	}

} // namespace ignis