#pragma once

#include "EditorPanel.h"
#include "Ignis/Scene/Scene.h"
#include "Ignis/Scene/Entity.h"

namespace ignis {

	class PropertiesPanel;

	class SceneHierarchyPanel : public EditorPanel
	{
	public:
		SceneHierarchyPanel() = default;
		~SceneHierarchyPanel() override = default;

		void OnImGuiRender() override;

		std::string_view GetName() const override { return "Scene Hierarchy"; }
		std::string_view GetID() const override { return "SceneHierarchy"; }

		void SetScene(std::shared_ptr<Scene> scene) { m_scene = scene; }
		void SetPropertiesPanel(PropertiesPanel* properties_panel) { m_properties_panel = properties_panel; }

	private:
		void DrawEntityNode(Entity entity);

	private:
		std::shared_ptr<Scene> m_scene;
		PropertiesPanel* m_properties_panel = nullptr;
		std::shared_ptr<Entity> m_selected_entity;
	};

} // namespace ignis