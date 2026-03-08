#pragma once

#include "Ignis.h"
#include "Editor/Panels/EditorPanel.h"

namespace ignis {

	// Properties panel for displaying and editing object properties
	class PropertiesPanel : public EditorPanel
	{
	public:
		PropertiesPanel() = default;
		~PropertiesPanel() = default;

		// EditorPanel interface
		void OnImGuiRender() override;
		std::string_view GetName() const override { return "Properties"; }
		std::string_view GetID() const override { return "Properties"; }
		
		// Set the entity to display properties for
		void SetSelectedEntity(std::shared_ptr<Entity> entity);
		std::shared_ptr<Entity> GetSelectedEntity() const { return m_selected_entity.lock(); }
		
		// Set the asset to display import settings for
		void SetSelectedAsset(AssetHandle handle);
		AssetHandle GetSelectedAsset() const { return m_selected_asset; }
		
		// Set current mesh for editing (demo approach)
		void SetCurrentMesh(std::shared_ptr<Mesh>* mesh, TransformComponent* transform = nullptr) 
		{ 
			m_current_mesh_ptr = mesh; 
			m_mesh_transform = transform;
		}
		void SetMeshTransform(TransformComponent* transform) { m_mesh_transform = transform; }
		
	private:
		void RenderTransformComponent(TransformComponent& transform);
		void RenderCameraComponent(CameraComponent& camera);
		void RenderDirectionalLightComponent(DirectionalLightComponent& light);
		void RenderPointLightComponent(PointLightComponent& light);
		void RenderSpotLightComponent(SpotLightComponent& light);
		void RenderSkyLightComponent(SkyLightComponent& light);
		void RenderMeshComponent(MeshComponent& mesh);
		void RenderScriptComponent(ScriptComponent& script);
		void RenderTextComponent(TextComponent& text);
		void RenderRectTransformComponent(RectTransformComponent& rect);
		void RenderCanvasComponent(CanvasComponent& canvas);
		void RenderImageComponent(ImageComponent& image);
		void RenderUITextComponent(UITextComponent& text);
		void RenderButtonComponent(ButtonComponent& button);
		void RenderProgressBarComponent(ProgressBarComponent& bar);
		void RenderAudioSourceComponent(AudioSourceComponent& audio);
		void RenderAudioListenerComponent(AudioListenerComponent& listener);
		
		// Mesh editing UI
		void RenderMeshEditor();
		void RenderMaterialsUI(std::shared_ptr<Mesh> mesh);
		void RenderTextureSlot(std::shared_ptr<Mesh> mesh, uint32_t material_index, MaterialType type, const char* label);
		void LoadNewModel(const std::string& filepath);
		void LoadMeshFromFile(const std::string& filepath, MeshComponent& mesh_component);
		void RenderTextureMapSlot(const char* label, AssetHandle& texture_handle, MeshComponent& mesh_component, MaterialType type);
		
		// Add Component UI
		void DrawAddComponentMenu(std::shared_ptr<Entity> entity);

		// Asset import settings rendering
		void RenderAssetProperties(AssetHandle handle);
		void RenderTextureImportSettings(TextureImportOptions& opts, AssetHandle handle);
		void RenderFontImportSettings(FontImportOptions& opts, AssetHandle handle);
		void RenderAudioImportSettings(AudioImportOptions& opts, AssetHandle handle);
		void RenderEquirectImportSettings(EquirectImportOptions& opts, AssetHandle handle);
		
		// Call this for saving asset import setting
		void ReimportAsset(AssetHandle handle);
		
	private:
		// TODO: Replace weak_ptr with EntityHandle + Scene* for safer entity management
		// Current approach uses weak_ptr to avoid dangling pointers, but handle-based
		std::weak_ptr<Entity> m_selected_entity;
		
		// Current mesh editing (demo approach - not ECS)
		std::shared_ptr<Mesh>* m_current_mesh_ptr = nullptr;
		TransformComponent* m_mesh_transform = nullptr;
		
		// Asset import settings
		AssetHandle m_selected_asset = AssetHandle::Invalid;
		bool m_asset_settings_modified = false;
		AssetImportOptions m_original_import_options; // Store original to detect revert so that the yellow sign can be removed
	};

} // namespace ignis