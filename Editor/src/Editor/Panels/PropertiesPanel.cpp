#include "Editor/Panels/PropertiesPanel.h"

#include "Ignis/Asset/AssetManager.h"
#include "Ignis/Project/Project.h"

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
			
			// Section 2: Show selected entity controls below
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
				
				// Render Camera Component
				if (entity->HasComponent<CameraComponent>())
				{
					auto& camera = entity->GetComponent<CameraComponent>();
					RenderCameraComponent(camera);
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
				
				// Render Mesh Component
				if (entity->HasComponent<MeshComponent>())
				{
					auto& mesh = entity->GetComponent<MeshComponent>();
					RenderMeshComponent(mesh);
				}
				
				// Render Script Component
				if (entity->HasComponent<ScriptComponent>())
				{
					auto& script = entity->GetComponent<ScriptComponent>();
					RenderScriptComponent(script);
				}
				
				// Render Text Component
				if (entity->HasComponent<TextComponent>())
				{
					auto& text = entity->GetComponent<TextComponent>();
					RenderTextComponent(text);
				}
				
				// Add Component button at bottom
				ImGui::Separator();
				ImGui::Spacing();
				
				float button_width = ImGui::GetContentRegionAvail().x;
				if (ImGui::Button("Add Component", ImVec2(button_width, 0)))
				{
					ImGui::OpenPopup("AddComponentPopup");
				}
				
				// Add Component popup menu
				if (ImGui::BeginPopup("AddComponentPopup"))
				{
					DrawAddComponentMenu(entity);
					ImGui::EndPopup();
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
				mesh_path = metadata->FilePath;
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
				preview = metadata->FilePath;
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
	
	void PropertiesPanel::RenderCameraComponent(CameraComponent& camera_component)
	{
		ImGui::PushID("CameraComponent");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Camera Component", flags);
		
		// Remove button
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		if (ImGui::Button("X", ImVec2(20, 20)))
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<CameraComponent>();
			}
		}
		
		if (open)
		{
			// Primary checkbox
			ImGui::Checkbox("Primary", &camera_component.Primary);
			
			// Fixed aspect ratio checkbox
			ImGui::Checkbox("Fixed Aspect Ratio", &camera_component.FixedAspectRatio);
			
			// Projection type dropdown
			const char* projection_types[] = { "Perspective", "Orthographic" };
			int current_projection = (int)camera_component.Camera->GetProjectionType();
			if (ImGui::Combo("Projection", &current_projection, projection_types, 2))
			{
				camera_component.Camera->SetProjectionType(
					(SceneCamera::ProjectionType)current_projection);
			}
			
			ImGui::Spacing();
			
			// Perspective settings
			if (camera_component.Camera->GetProjectionType() == SceneCamera::ProjectionType::Perspective)
			{
				float fov = camera_component.Camera->GetPerspectiveFOV();
				if (ImGui::SliderFloat("FOV", &fov, 1.0f, 120.0f))
				{
					camera_component.Camera->SetPerspectiveFOV(fov);
				}
				
				float near_clip = camera_component.Camera->GetPerspectiveNearClip();
				if (ImGui::DragFloat("Near Clip", &near_clip, 0.01f, 0.001f, 100.0f))
				{
					camera_component.Camera->SetPerspectiveNearClip(near_clip);
				}
				
				float far_clip = camera_component.Camera->GetPerspectiveFarClip();
				if (ImGui::DragFloat("Far Clip", &far_clip, 1.0f, 1.0f, 10000.0f))
				{
					camera_component.Camera->SetPerspectiveFarClip(far_clip);
				}
			}
			// Orthographic settings
			else
			{
				float size = camera_component.Camera->GetOrthographicSize();
				if (ImGui::SliderFloat("Size", &size, 0.1f, 100.0f))
				{
					camera_component.Camera->SetOrthographicSize(size);
				}
				
				float near_clip = camera_component.Camera->GetOrthographicNearClip();
				if (ImGui::DragFloat("Near Clip", &near_clip, 0.1f, -100.0f, 100.0f))
				{
					camera_component.Camera->SetOrthographicNearClip(near_clip);
				}
				
				float far_clip = camera_component.Camera->GetOrthographicFarClip();
				if (ImGui::DragFloat("Far Clip", &far_clip, 0.1f, -100.0f, 100.0f))
				{
					camera_component.Camera->SetOrthographicFarClip(far_clip);
				}
			}
			
			ImGui::Spacing();
		}
		
		ImGui::PopID();
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
			// Environment Asset section
			ImGui::Text("Environment:");
			ImGui::Indent();
			
			// Display current environment info
			if (light.SceneEnvironment.IsValid())
			{
				if (auto* metadata = AssetManager::GetMetadata(light.SceneEnvironment))
				{
					ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Loaded");
					ImGui::SameLine();
					ImGui::TextWrapped("%s", metadata->FilePath.c_str());
				}
				else
				{
					ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "Invalid Handle");
					ImGui::SameLine();
					ImGui::Text("%llu", (uint64_t)light.SceneEnvironment);
				}
			}
			else
			{
				ImGui::TextDisabled("No environment loaded");
			}
			
			// Browse button
			if (ImGui::Button("Browse Environment...", ImVec2(-1, 0)))
			{
				std::string filepath = FileDialog::OpenFile("HDR Environment", {"hdr"});
				if (!filepath.empty())
				{
					// Import environment asset
					AssetHandle env_handle = AssetManager::ImportAsset(filepath);
					if (env_handle.IsValid())
					{
						light.SceneEnvironment = env_handle;
						
						// Save asset registry
						AssetManager::SaveAssetRegistry(Project::GetActiveAssetRegistry());
						
						Log::CoreInfo("PropertiesPanel: Loaded environment '{}'", 
						              std::filesystem::path(filepath).filename().string());
					}
					else
					{
						Log::Error("Failed to import environment: {}", filepath);
					}
				}
			}
			
			ImGui::Unindent();
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();
			
			// Environment settings
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

	void PropertiesPanel::RenderMeshComponent(MeshComponent& mesh_component)
	{
		ImGui::PushID("MeshComponent");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Mesh Component", flags);
		
		// Remove button
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		if (ImGui::Button("X", ImVec2(20, 20)))
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<MeshComponent>();
			}
		}
		
		if (open)
		{
			// Display mesh info
			if (mesh_component.Mesh.IsValid())
			{
				ImGui::Text("Mesh Handle: %llu", mesh_component.Mesh);
				
				// Show mesh file path if available
				if (auto* metadata = AssetManager::GetMetadata(mesh_component.Mesh))
				{
					ImGui::TextWrapped("File: %s", std::filesystem::path(metadata->FilePath).filename().string().c_str());
				}
			}
			else
			{
				ImGui::TextDisabled("No mesh loaded");
			}
			
			// Browse Model button
			if (ImGui::Button("Browse Model...", ImVec2(-1, 0)))
			{
				std::string filepath = FileDialog::OpenFile("3D Models", {"obj", "fbx", "gltf", "glb"});
				if (!filepath.empty())
				{
					LoadMeshFromFile(filepath, mesh_component);
				}
			}
			
			ImGui::Separator();
			ImGui::Spacing();
			
			// Material/Texture section
			if (ImGui::TreeNodeEx("Material Textures", ImGuiTreeNodeFlags_DefaultOpen))
			{
				RenderTextureMapSlot("Albedo Map", mesh_component.MeterialData.AlbedoMap, mesh_component, MaterialType::Albedo);
				RenderTextureMapSlot("Normal Map", mesh_component.MeterialData.NormalMap, mesh_component, MaterialType::Normal);
				RenderTextureMapSlot("Metalness Map", mesh_component.MeterialData.MetalnessMap, mesh_component, MaterialType::Metal);
				RenderTextureMapSlot("Roughness Map", mesh_component.MeterialData.RoughnessMap, mesh_component, MaterialType::Roughness);
				RenderTextureMapSlot("Emissive Map", mesh_component.MeterialData.EmissiveMap, mesh_component, MaterialType::Emissive);
				RenderTextureMapSlot("AO Map", mesh_component.MeterialData.AOMap, mesh_component, MaterialType::AO);
				
				ImGui::TreePop();
			}
			
			ImGui::Spacing();
		}
		
		ImGui::PopID();
	}

	void PropertiesPanel::RenderScriptComponent(ScriptComponent& script_component)
	{
		ImGui::PushID("ScriptComponent");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Script Component", flags);
		
		// Remove button
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		if (ImGui::Button("X", ImVec2(20, 20)))
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<ScriptComponent>();
			}
		}
		
		if (open)
		{
			// Script class name input
			char buffer[256];
			strncpy(buffer, script_component.ClassName.c_str(), sizeof(buffer) - 1);
			buffer[sizeof(buffer) - 1] = '\0';
			
			if (ImGui::InputText("Class Name", buffer, sizeof(buffer)))
			{
				script_component.ClassName = buffer;
			}
			
			// Enabled checkbox
			ImGui::Checkbox("Enabled", &script_component.Enabled);
			
			ImGui::Spacing();
		}
		
		ImGui::PopID();
	}

	void PropertiesPanel::RenderTextComponent(TextComponent& text_component)
	{
		ImGui::PushID("TextComponent");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Text Component", flags);
		
		// Remove button
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		if (ImGui::Button("X", ImVec2(20, 20)))
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<TextComponent>();
			}
		}
		
		if (open)
		{
			// Text input (multiline)
			char buffer[1024];
			strncpy(buffer, text_component.Text.c_str(), sizeof(buffer) - 1);
			buffer[sizeof(buffer) - 1] = '\0';
			
			if (ImGui::InputTextMultiline("Text", buffer, sizeof(buffer), ImVec2(-1, 100)))
			{
				text_component.Text = buffer;
			}
			
			// Font asset selector
			ImGui::Text("Font:");
			ImGui::Indent();
			if (text_component.Font.IsValid())
			{
				ImGui::Text("Asset: %s", text_component.Font.ToString().c_str());
			}
			else
			{
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No font assigned");
			}
			
			if (ImGui::Button("Select Font", ImVec2(-1, 0)))
			{
				// TODO: Open font asset picker when asset browser is ready
				Log::CoreWarn("Font asset picker not yet implemented");
			}
			ImGui::Unindent();
			
			// Color picker
			ImGui::ColorEdit3("Color", &text_component.Color[0]);
			
			// Alpha slider
			ImGui::SliderFloat("Alpha", &text_component.Alpha, 0.0f, 1.0f);
			
			// Scale slider
			ImGui::SliderFloat("Scale", &text_component.Scale, 0.1f, 10.0f);
			
			ImGui::Spacing();
		}
		
		ImGui::PopID();
	}

	void PropertiesPanel::RenderTextureMapSlot(const char* label, AssetHandle& texture_handle, MeshComponent& mesh_component, MaterialType type)
	{
		ImGui::PushID(label);
		
		// Label
		ImGui::Text("%s:", label);
		ImGui::Indent();
		
		// Display texture info
		if (texture_handle.IsValid())
		{
			if (auto* metadata = AssetManager::GetMetadata(texture_handle))
			{
				ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Loaded");
				ImGui::SameLine();
				ImGui::TextWrapped("%s", std::filesystem::path(metadata->FilePath).filename().string().c_str());
			}
			else
			{
				ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.4f, 1.0f), "Handle: %llu", texture_handle);
			}
		}
		else
		{
			ImGui::TextDisabled("Not loaded");
		}
		
		// Browse button
		if (ImGui::Button("Browse...", ImVec2(-1, 0)))
		{
			std::string filepath = FileDialog::OpenFile("Texture Files", {"png", "jpg", "jpeg", "tga", "bmp"});
			if (!filepath.empty())
			{
				// Import texture
				AssetHandle new_texture_handle = AssetManager::ImportAsset(filepath);
				if (new_texture_handle.IsValid())
				{
					// Update the texture handle
					texture_handle = new_texture_handle;
					
					// Update mesh material data if mesh is loaded
					if (mesh_component.Mesh.IsValid())
					{
						auto mesh = AssetManager::GetAsset<Mesh>(mesh_component.Mesh);
						if (mesh && !mesh->GetMaterialsData().empty())
						{
							mesh->SetMaterialDataTexture(0, type, new_texture_handle);
						}
					}
					
					// Save asset registry to persist the imported texture
					AssetManager::SaveAssetRegistry(Project::GetActiveAssetRegistry());
					
					Log::CoreInfo("PropertiesPanel: Loaded texture '{}' for {}", 
					              std::filesystem::path(filepath).filename().string(), 
					              label);
				}
				else
				{
					Log::Error("Failed to import texture: {}", filepath);
				}
			}
		}
		
		ImGui::Unindent();
		ImGui::Spacing();
		ImGui::PopID();
	}

	void PropertiesPanel::LoadMeshFromFile(const std::string& filepath, MeshComponent& mesh_component)
	{
		// Validate file extension
		std::filesystem::path path(filepath);
		std::string extension = path.extension().string();
		
		// Convert to lowercase for comparison
		std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
		
		if (extension != ".obj" && extension != ".fbx" && extension != ".gltf" && extension != ".glb")
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
		
		// Save asset registry to persist the imported mesh
		AssetManager::SaveAssetRegistry(Project::GetActiveAssetRegistry());
		
		// Load mesh to verify it's valid
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
		
		// Assign mesh handle to component
		mesh_component.Mesh = mesh_handle;
		
		// Initialize materials with default textures for missing slots
		const auto& materials = new_mesh->GetMaterialsData();
		Log::CoreInfo("Loaded model with {} materials: {}", materials.size(), filepath);
		
		for (uint32_t i = 0; i < materials.size(); ++i)
		{
			const MaterialData& mat = materials[i];
			
			// Set default textures for missing material slots
			if (!mat.AlbedoMap.IsValid())
			{
				new_mesh->SetMaterialDataTexture(i, MaterialType::Albedo, 
				                                      Renderer::GetWhiteTextureHandle());
			}
			
			if (!mat.NormalMap.IsValid())
			{
				new_mesh->SetMaterialDataTexture(i, MaterialType::Normal, 
				                                      Renderer::GetDefaultNormalTextureHandle());
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
		
		// Copy material data back to component (only if materials exist)
		if (!new_mesh->GetMaterialsData().empty())
		{
			mesh_component.MeterialData = new_mesh->GetMaterialsData()[0];
		}
		else
		{
			// Initialize with default material data
			mesh_component.MeterialData = MaterialData{};
			Log::CoreWarn("Mesh has no materials, using default material data");
		}
		
		if (auto entity = m_selected_entity.lock())
		{
			Log::CoreInfo("PropertiesPanel: Loaded mesh '{}' into entity '{}'", 
			              path.filename().string(),
			              entity->GetComponent<TagComponent>().Tag);
		}
	}

	// Template helper function for drawing add component menu items
	template<typename T>
	static void DrawAddComponentMenuItemImpl(std::shared_ptr<Entity> entity, const char* name)
	{
		// Don't show if entity already has this component
		if (entity->HasComponent<T>())
			return;
		
		if (ImGui::MenuItem(name))
		{
			entity->AddComponent<T>();
			Log::CoreInfo("PropertiesPanel: Added {} to entity '{}'", 
						  name, 
						  entity->GetComponent<TagComponent>().Tag);
			ImGui::CloseCurrentPopup();
		}
	}

	void PropertiesPanel::DrawAddComponentMenu(std::shared_ptr<Entity> entity)
	{
		ImGui::TextDisabled("Select Component to Add:");
		ImGui::Separator();
		
		// Camera Component
		ImGui::Text("Camera:");
		DrawAddComponentMenuItemImpl<CameraComponent>(entity, "  Camera");
		
		ImGui::Separator();
		
		// Light Components
		ImGui::Text("Lights:");
		DrawAddComponentMenuItemImpl<DirectionalLightComponent>(entity, "  Directional Light");
		DrawAddComponentMenuItemImpl<PointLightComponent>(entity, "  Point Light");
		DrawAddComponentMenuItemImpl<SpotLightComponent>(entity, "  Spot Light");
		DrawAddComponentMenuItemImpl<SkyLightComponent>(entity, "  Sky Light");
		
		ImGui::Separator();
		
		// Rendering Components
		ImGui::Text("Rendering:");
		DrawAddComponentMenuItemImpl<MeshComponent>(entity, "  Mesh");
		DrawAddComponentMenuItemImpl<TextComponent>(entity, "  Text");
		
		ImGui::Separator();
		
		// Script Component
		ImGui::Text("Scripting:");
		DrawAddComponentMenuItemImpl<ScriptComponent>(entity, "  Script");
	}

	void PropertiesPanel::ReimportAsset(AssetHandle handle)
	{
		if (AssetMetadata* meta = AssetManager::GetMetadataMutable(handle))
		{
			std::visit(overloaded{
				[](std::monostate) {
					// TODO
				},
				[](TextureImportOptions& opts) {
					// TODO
				},
				[](FontImportOptions& opts) {
					// TODO
				},
				[](AudioImportOptions& opts) {
					// TODO
				},
				[](EquirectImportOptions& opts) {
					// TODO
				},
				}, meta->ImportOptions);
		}

		AssetManager::UnloadAsset(handle);
		AssetManager::SaveAssetRegistry(Project::GetActiveAssetRegistry());
	}

} // namespace ignis