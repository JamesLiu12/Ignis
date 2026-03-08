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
			
			// Section 2: Check asset selection first (mutually exclusive with entity)
			if (m_selected_asset.IsValid())
			{
				RenderAssetProperties(m_selected_asset);
			}
			// Section 3: Show selected entity controls if no asset selected
			else if (auto entity = m_selected_entity.lock())
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
				
				// Render RectTransform Component
				if (entity->HasComponent<RectTransformComponent>())
				{
					auto& rect = entity->GetComponent<RectTransformComponent>();
					RenderRectTransformComponent(rect);
				}
				
				// Render Canvas Component
				if (entity->HasComponent<CanvasComponent>())
				{
					auto& canvas = entity->GetComponent<CanvasComponent>();
					RenderCanvasComponent(canvas);
				}
				
				// Render Image Component
				if (entity->HasComponent<ImageComponent>())
				{
					auto& image = entity->GetComponent<ImageComponent>();
					RenderImageComponent(image);
				}
				
				// Render UIText Component
				if (entity->HasComponent<UITextComponent>())
				{
					auto& ui_text = entity->GetComponent<UITextComponent>();
					RenderUITextComponent(ui_text);
				}
				
				// Render Button Component
				if (entity->HasComponent<ButtonComponent>())
				{
					auto& button = entity->GetComponent<ButtonComponent>();
					RenderButtonComponent(button);
				}
				
				// Render ProgressBar Component
				if (entity->HasComponent<ProgressBarComponent>())
				{
					auto& bar = entity->GetComponent<ProgressBarComponent>();
					RenderProgressBarComponent(bar);
				}
				
				// Render AudioSource Component
				if (entity->HasComponent<AudioSourceComponent>())
				{
					auto& audio = entity->GetComponent<AudioSourceComponent>();
					RenderAudioSourceComponent(audio);
				}
				
				// Render AudioListener Component
				if (entity->HasComponent<AudioListenerComponent>())
				{
					auto& listener = entity->GetComponent<AudioListenerComponent>();
					RenderAudioListenerComponent(listener);
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
			ImGui::DragFloat("Intensity", &light.Intensity, 0.1f, 0.0f, 1000.0f, "%.2f");
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
			ImGui::DragFloat("Intensity", &light.Intensity, 0.1f, 0.0f, 1000.0f, "%.2f");
			ImGui::DragFloat("Range", &light.Range, 0.5f, 0.1f, 10000.0f, "%.1f");
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
			ImGui::DragFloat("Intensity", &light.Intensity, 0.1f, 0.0f, 1000.0f, "%.2f");
			ImGui::DragFloat("Range", &light.Range, 0.5f, 0.1f, 10000.0f, "%.1f");
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
			ImGui::DragFloat("Intensity", &light.Intensity, 0.1f, 0.0f, 1000.0f, "%.2f");
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
				if (auto* metadata = AssetManager::GetMetadata(mesh_component.Mesh))
				{
					ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Loaded");
					ImGui::SameLine();
					ImGui::TextWrapped("%s", metadata->FilePath.c_str());
				}
				else
				{
					ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "Invalid Handle");
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
				if (auto* metadata = AssetManager::GetMetadata(text_component.Font))
				{
					ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Loaded");
					ImGui::SameLine();
					ImGui::TextWrapped("%s", metadata->FilePath.c_str());
				}
				else
				{
					ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "Invalid Handle");
				}
			}
			else
			{
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No font assigned");
			}
			
			if (ImGui::Button("Select Font", ImVec2(-1, 0)))
			{
				std::string filepath = FileDialog::OpenFile("Font Files", {"ttf", "otf"});
				if (!filepath.empty())
				{
					// Import font asset
					AssetHandle handle = AssetManager::ImportAsset(filepath);
					if (handle.IsValid())
					{
						text_component.Font = handle;
						Log::CoreInfo("Loaded font: {}", filepath);
					}
				}
			}
			ImGui::Unindent();
			
			// Color picker
			ImGui::ColorEdit3("Color", &text_component.Color[0]);
		
			// Alpha slider
			ImGui::SliderFloat("Alpha", &text_component.Alpha, 0.0f, 1.0f);
		
			// Scale (unbounded)
			ImGui::DragFloat("Scale", &text_component.Scale, 0.01f, 0.01f, FLT_MAX, "%.2f");
			
			ImGui::Spacing();
		}
		
		ImGui::PopID();
	}

	void PropertiesPanel::RenderRectTransformComponent(RectTransformComponent& rect)
	{
		ImGui::PushID("RectTransformComponent");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Rect Transform", flags);
		
		// Remove button
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		if (ImGui::Button("X", ImVec2(20, 20)))
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<RectTransformComponent>();
			}
		}
		
		if (open)
		{
			// Anchor Min
			ImGui::DragFloat2("Anchor Min", &rect.AnchorMin[0], 0.01f, 0.0f, 1.0f);
			
			// Anchor Max
			ImGui::DragFloat2("Anchor Max", &rect.AnchorMax[0], 0.01f, 0.0f, 1.0f);
			
			// Offset Min
			ImGui::DragFloat2("Offset Min", &rect.OffsetMin[0], 1.0f);
			
			// Offset Max
			ImGui::DragFloat2("Offset Max", &rect.OffsetMax[0], 1.0f);
			
			// Show resolved values (read-only)
			ImGui::Separator();
			ImGui::TextDisabled("Resolved (Read-Only):");
			ImGui::Text("Min: (%.1f, %.1f)", rect.ResolvedMin.x, rect.ResolvedMin.y);
			ImGui::Text("Max: (%.1f, %.1f)", rect.ResolvedMax.x, rect.ResolvedMax.y);
			ImGui::Text("Size: (%.1f, %.1f)", rect.GetSize().x, rect.GetSize().y);
			
			ImGui::Spacing();
		}
		
		ImGui::PopID();
	}

	void PropertiesPanel::RenderCanvasComponent(CanvasComponent& canvas)
	{
		ImGui::PushID("CanvasComponent");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Canvas", flags);
		
		// Remove button
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		if (ImGui::Button("X", ImVec2(20, 20)))
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<CanvasComponent>();
			}
		}
		
		if (open)
		{
			// Render Mode dropdown
			const char* render_modes[] = { "Screen Space", "World Space" };
			int current_mode = static_cast<int>(canvas.Mode);
			if (ImGui::Combo("Render Mode", &current_mode, render_modes, 2))
			{
				canvas.Mode = static_cast<CanvasComponent::RenderMode>(current_mode);
			}
			
			// Sort Order
			ImGui::DragInt("Sort Order", &canvas.SortOrder, 1.0f, -100, 100);
			ImGui::TextDisabled("Higher values render on top");
			
			// Visible checkbox
			ImGui::Checkbox("Visible", &canvas.Visible);
			
			ImGui::Spacing();
		}
		
		ImGui::PopID();
	}

	void PropertiesPanel::RenderImageComponent(ImageComponent& image)
	{
		ImGui::PushID("ImageComponent");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Image", flags);
		
		// Remove button
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		if (ImGui::Button("X", ImVec2(20, 20)))
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<ImageComponent>();
			}
		}
		
		if (open)
		{
			// Texture asset selector
			ImGui::Text("Texture:");
			ImGui::Indent();
			if (image.Texture.IsValid())
			{
				if (auto* metadata = AssetManager::GetMetadata(image.Texture))
				{
					ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Loaded");
					ImGui::SameLine();
					ImGui::TextWrapped("%s", metadata->FilePath.c_str());
				}
				else
				{
					ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "Invalid Handle");
				}
			}
			else
			{
				ImGui::TextDisabled("No texture (solid color)");
			}
			
			if (ImGui::Button("Select Texture", ImVec2(-1, 0)))
			{
				std::string filepath = FileDialog::OpenFile("Texture Files", {"png", "jpg", "jpeg", "tga", "bmp"});
				if (!filepath.empty())
				{
					// Import texture asset
					AssetHandle handle = AssetManager::ImportAsset(filepath);
					if (handle.IsValid())
					{
						image.Texture = handle;
						Log::CoreInfo("Loaded texture: {}", filepath);
					}
				}
			}
			ImGui::Unindent();
			
			// Color picker
			ImGui::ColorEdit4("Color", &image.Color[0]);
			
			// Scale Mode dropdown
			const char* scale_modes[] = { "Stretch", "Fit Inside", "Fit Outside", "Native Size" };
			int current_scale = static_cast<int>(image.Scale);
			if (ImGui::Combo("Scale Mode", &current_scale, scale_modes, 4))
			{
				image.Scale = static_cast<ImageComponent::ScaleMode>(current_scale);
			}
			
			// Visible checkbox
			ImGui::Checkbox("Visible", &image.Visible);
			
			// Raycast Target checkbox
			ImGui::Checkbox("Raycast Target", &image.RaycastTarget);
			ImGui::TextDisabled("Participates in UI hit-testing");
			
			ImGui::Spacing();
		}
		
		ImGui::PopID();
	}

	void PropertiesPanel::RenderUITextComponent(UITextComponent& ui_text)
	{
		ImGui::PushID("UITextComponent");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("UI Text", flags);
		
		// Remove button
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		if (ImGui::Button("X", ImVec2(20, 20)))
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<UITextComponent>();
			}
		}
		
		if (open)
		{
			// Text input (multiline)
			char buffer[1024];
			strncpy(buffer, ui_text.Text.c_str(), sizeof(buffer) - 1);
			buffer[sizeof(buffer) - 1] = '\0';
			
			if (ImGui::InputTextMultiline("Text", buffer, sizeof(buffer), ImVec2(-1, 100)))
			{
				ui_text.Text = buffer;
			}
			
			// Font asset selector
			ImGui::Text("Font:");
			ImGui::Indent();
			if (ui_text.Font.IsValid())
			{
				if (auto* metadata = AssetManager::GetMetadata(ui_text.Font))
				{
					ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Loaded");
					ImGui::SameLine();
					ImGui::TextWrapped("%s", metadata->FilePath.c_str());
				}
				else
				{
					ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "Invalid Handle");
				}
			}
			else
			{
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No font assigned");
			}
			
			if (ImGui::Button("Select Font", ImVec2(-1, 0)))
			{
				std::string filepath = FileDialog::OpenFile("Font Files", {"ttf", "otf"});
				if (!filepath.empty())
				{
					// Import font asset
					AssetHandle handle = AssetManager::ImportAsset(filepath);
					if (handle.IsValid())
					{
						ui_text.Font = handle;
						Log::CoreInfo("Loaded font: {}", filepath);
					}
				}
			}
			ImGui::Unindent();
			
			// Color picker
			ImGui::ColorEdit4("Color", &ui_text.Color[0]);
			
			// Font Size
			ImGui::DragFloat("Font Size", &ui_text.FontSize, 0.5f, 1.0f, 200.0f);
			
			// Horizontal Alignment
			const char* h_align[] = { "Left", "Center", "Right" };
			int current_h_align = static_cast<int>(ui_text.HAlign);
			if (ImGui::Combo("Horizontal Align", &current_h_align, h_align, 3))
			{
				ui_text.HAlign = static_cast<UITextComponent::HorizontalAlignment>(current_h_align);
			}
			
			// Vertical Alignment
			const char* v_align[] = { "Top", "Middle", "Bottom" };
			int current_v_align = static_cast<int>(ui_text.VAlign);
			if (ImGui::Combo("Vertical Align", &current_v_align, v_align, 3))
			{
				ui_text.VAlign = static_cast<UITextComponent::VerticalAlignment>(current_v_align);
			}
			
			// Visible checkbox
			ImGui::Checkbox("Visible", &ui_text.Visible);
			
			ImGui::Spacing();
		}
		
		ImGui::PopID();
	}

	void PropertiesPanel::RenderButtonComponent(ButtonComponent& button)
	{
		ImGui::PushID("ButtonComponent");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Button", flags);
		
		// Remove button
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		if (ImGui::Button("X", ImVec2(20, 20)))
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<ButtonComponent>();
			}
		}
		
		if (open)
		{
			// Interactable checkbox
			ImGui::Checkbox("Interactable", &button.Interactable);
			
			ImGui::Separator();
			ImGui::Text("Colors:");
			
			// Normal Color
			ImGui::ColorEdit4("Normal", &button.NormalColor[0]);
			
			// Hover Color
			ImGui::ColorEdit4("Hover", &button.HoverColor[0]);
			
			// Pressed Color
			ImGui::ColorEdit4("Pressed", &button.PressedColor[0]);
			
			// Disabled Color
			ImGui::ColorEdit4("Disabled", &button.DisabledColor[0]);
			
			// Runtime state (read-only)
			ImGui::Separator();
			ImGui::TextDisabled("Runtime State (Read-Only):");
			ImGui::Text("Is Hovered: %s", button.IsHovered ? "Yes" : "No");
			ImGui::Text("Is Pressed: %s", button.IsPressed ? "Yes" : "No");
			
			ImGui::Spacing();
		}
		
		ImGui::PopID();
	}

	void PropertiesPanel::RenderProgressBarComponent(ProgressBarComponent& bar)
	{
		ImGui::PushID("ProgressBarComponent");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Progress Bar", flags);
		
		// Remove button
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		if (ImGui::Button("X", ImVec2(20, 20)))
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<ProgressBarComponent>();
			}
		}
		
		if (open)
		{
			// Value slider
			ImGui::SliderFloat("Value", &bar.Value, bar.MinValue, bar.MaxValue);
			
			// Min/Max values
			ImGui::DragFloat("Min Value", &bar.MinValue, 0.1f);
			ImGui::DragFloat("Max Value", &bar.MaxValue, 0.1f);
			
			// Show normalized value
			ImGui::Text("Normalized: %.2f%%", bar.GetNormalizedValue() * 100.0f);
			
			ImGui::Separator();
			
			// Foreground Color
			ImGui::ColorEdit4("Foreground", &bar.ForegroundColor[0]);
			
			// Background Color
			ImGui::ColorEdit4("Background", &bar.BackgroundColor[0]);
			
			// Fill Direction
			const char* directions[] = { "Left to Right", "Right to Left", "Bottom to Top", "Top to Bottom" };
			int current_direction = static_cast<int>(bar.Direction);
			if (ImGui::Combo("Fill Direction", &current_direction, directions, 4))
			{
				bar.Direction = static_cast<ProgressBarComponent::FillDirection>(current_direction);
			}
			
			// Visible checkbox
			ImGui::Checkbox("Visible", &bar.Visible);
			
			ImGui::Spacing();
		}
		
		ImGui::PopID();
	}

	void PropertiesPanel::RenderAudioSourceComponent(AudioSourceComponent& audio)
	{
		ImGui::PushID("AudioSourceComponent");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Audio Source", flags);
		
		// Remove button
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		if (ImGui::Button("X", ImVec2(20, 20)))
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<AudioSourceComponent>();
			}
		}
		
		if (open)
		{
			// Audio Clip asset selector
			ImGui::Text("Audio Clip:");
			ImGui::Indent();
			if (audio.Clip.IsValid())
			{
				if (auto* metadata = AssetManager::GetMetadata(audio.Clip))
				{
					ImGui::TextColored(ImVec4(0.4f, 0.8f, 0.4f, 1.0f), "Loaded");
					ImGui::SameLine();
					ImGui::TextWrapped("%s", metadata->FilePath.c_str());
				}
				else
				{
					ImGui::TextColored(ImVec4(0.8f, 0.4f, 0.4f, 1.0f), "Invalid Handle");
				}
			}
			else
			{
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "No audio clip assigned");
			}
			
			if (ImGui::Button("Select Audio Clip", ImVec2(-1, 0)))
			{
				std::string filepath = FileDialog::OpenFile("Audio Files", {"wav", "mp3", "flac", "ogg"});
				if (!filepath.empty())
				{
					// Import audio asset
					AssetHandle handle = AssetManager::ImportAsset(filepath);
					if (handle.IsValid())
					{
						audio.Clip = handle;
						Log::CoreInfo("Loaded audio clip: {}", filepath);
					}
				}
			}
			ImGui::Unindent();
			
			ImGui::Separator();
		
			// Volume
			ImGui::DragFloat("Volume", &audio.Volume, 0.01f, 0.0f, 10.0f, "%.2f");
		
			// Pitch
			ImGui::DragFloat("Pitch", &audio.Pitch, 0.01f, 0.1f, 10.0f, "%.2f");
		
			// Loop checkbox
			ImGui::Checkbox("Loop", &audio.Loop);
			
			// Play On Start checkbox
			ImGui::Checkbox("Play On Start", &audio.PlayOnStart);
			
			ImGui::Separator();
			
			// Spatial checkbox
			ImGui::Checkbox("Spatial (3D)", &audio.Spatial);
			
			if (audio.Spatial)
			{
				ImGui::Indent();
				
				// Min Distance
				ImGui::DragFloat("Min Distance", &audio.MinDistance, 0.1f, 0.0f, 1000.0f);
				ImGui::TextDisabled("Distance where volume is at maximum");
				
				// Max Distance
				ImGui::DragFloat("Max Distance", &audio.MaxDistance, 1.0f, 0.0f, 10000.0f);
				ImGui::TextDisabled("Distance where volume reaches zero");
				
				ImGui::Unindent();
			}
			
			ImGui::Spacing();
		}
		
		ImGui::PopID();
	}

	void PropertiesPanel::RenderAudioListenerComponent(AudioListenerComponent& listener)
	{
		ImGui::PushID("AudioListenerComponent");
		
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap;
		bool open = ImGui::CollapsingHeader("Audio Listener", flags);
		
		// Remove button
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - 20);
		if (ImGui::Button("X", ImVec2(20, 20)))
		{
			if (auto entity = m_selected_entity.lock())
			{
				entity->RemoveComponent<AudioListenerComponent>();
			}
		}
		
		if (open)
		{
			// Primary checkbox
			ImGui::Checkbox("Primary", &listener.Primary);
			ImGui::TextDisabled("Only one listener should be primary");
			
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
		
		// UI Components
		ImGui::Text("UI:");
		DrawAddComponentMenuItemImpl<RectTransformComponent>(entity, "  Rect Transform");
		DrawAddComponentMenuItemImpl<CanvasComponent>(entity, "  Canvas");
		DrawAddComponentMenuItemImpl<ImageComponent>(entity, "  Image");
		DrawAddComponentMenuItemImpl<UITextComponent>(entity, "  UI Text");
		DrawAddComponentMenuItemImpl<ButtonComponent>(entity, "  Button");
		DrawAddComponentMenuItemImpl<ProgressBarComponent>(entity, "  Progress Bar");
		
		ImGui::Separator();
		
		// Audio Components
		ImGui::Text("Audio:");
		DrawAddComponentMenuItemImpl<AudioSourceComponent>(entity, "  Audio Source");
		DrawAddComponentMenuItemImpl<AudioListenerComponent>(entity, "  Audio Listener");
		
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
				[&](std::monostate&) {
					Log::CoreWarn("ReimportAsset: Asset has no import options");
				},
				[&](TextureImportOptions& opts) {
					Log::CoreInfo("ReimportAsset: Texture '{}' - FlipVertical={}, GenMipmaps={}, Format={}", 
						meta->FilePath, opts.FlipVertical, opts.GenMipmaps, (int)opts.InternalFormat);
				},
				[&](FontImportOptions& opts) {
					Log::CoreInfo("ReimportAsset: Font '{}' - Size={}, Atlas={}x{}", 
						meta->FilePath, opts.FontSize, opts.AtlasWidth, opts.AtlasHeight);
				},
				[&](AudioImportOptions& opts) {
					Log::CoreInfo("ReimportAsset: Audio '{}' - Stream={}", 
						meta->FilePath, opts.Stream);
				},
				[&](EquirectImportOptions& opts) {
					Log::CoreInfo("ReimportAsset: Equirect '{}' - EnvRes={}, IrrRes={}, PrefilterRes={}", 
						meta->FilePath, opts.BakeSettings.EnvironmentResolution, 
						opts.BakeSettings.IrradianceResolution, opts.BakeSettings.PrefilterResolution);
				},
				}, meta->ImportOptions);
			
			// Unload the asset to force reload with new settings
			AssetManager::UnloadAsset(handle);
			
			// Save the updated asset registry
			if (AssetManager::SaveAssetRegistry(Project::GetActiveAssetRegistry()))
			{
				Log::CoreInfo("ReimportAsset: Successfully saved asset registry");
			}
			else
			{
				Log::CoreError("ReimportAsset: Failed to save asset registry");
			}
		}
		else
		{
			Log::CoreError("ReimportAsset: Failed to get mutable metadata for handle {}", (uint64_t)handle);
		}
	}

} // namespace ignis