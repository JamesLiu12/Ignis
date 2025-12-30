#pragma once

#include "EditorPanel.h"
#include "Ignis/Scene/Entity.h"
#include "Ignis/Scene/Components.h"

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
		
	private:
		void RenderTransformComponent(TransformComponent& transform);
		void RenderLightComponent(LightComponent& light);
		
	private:
		// TODO: Replace weak_ptr with EntityHandle + Scene* for safer entity management
		// Current approach uses weak_ptr to avoid dangling pointers, but handle-based
		std::weak_ptr<Entity> m_selected_entity;
	};

} // namespace ignis