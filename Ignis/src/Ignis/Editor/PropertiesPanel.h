#pragma once

#include "EditorPanel.h"
#include "Ignis/Scene/Entity.h"
#include "Ignis/Scene/Components.h"
#include "Ignis/Renderer/Mesh.h"

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
		void SetSelectedEntity(std::shared_ptr<Entity> entity) { m_selected_entity = entity; }
		std::shared_ptr<Entity> GetSelectedEntity() const { return m_selected_entity.lock(); }
		
		// Set current mesh for editing (demo approach)
		void SetCurrentMesh(std::shared_ptr<Mesh> mesh) { m_current_mesh = mesh; }
		void SetMeshTransform(TransformComponent* transform) { m_mesh_transform = transform; }
		
	private:
		void RenderTransformComponent(TransformComponent& transform);
		void RenderDirectionalLightComponent(DirectionalLightComponent& light);
		void RenderPointLightComponent(PointLightComponent& light);
		void RenderSpotLightComponent(SpotLightComponent& light);
		
		// Mesh editing UI
		void RenderMeshEditor();
		void RenderMaterialsUI(std::shared_ptr<Mesh> mesh);
		void RenderTextureSlot(std::shared_ptr<Mesh> mesh, uint32_t material_index, 
		                       MaterialType type, const char* label);
		
	private:
		// TODO: Replace weak_ptr with EntityHandle + Scene* for safer entity management
		// Current approach uses weak_ptr to avoid dangling pointers, but handle-based
		std::weak_ptr<Entity> m_selected_entity;
		
		// Current mesh editing (demo approach - not ECS)
		std::shared_ptr<Mesh> m_current_mesh;
		TransformComponent* m_mesh_transform = nullptr;
	};

} // namespace ignis