#pragma once

#include "Ignis.h"
#include "Editor/Panels/EditorPanel.h"

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

		Entity GetSelectedEntity() { return m_selected_entity; }

	// Copy/Paste/Duplicate operations
	void CopySelectedEntity();
	void PasteEntity();
	void DuplicateSelectedEntity();

	private:
		void DrawEntityNode(Entity entity);
		void DrawEntityCreateMenu(Entity parent = Entity());

	private:
		std::shared_ptr<Scene> m_scene;
		PropertiesPanel* m_properties_panel = nullptr;
		Entity m_selected_entity;
		Entity m_renaming_entity;
		char m_rename_buffer[256] = "";

	// Copy/paste support
	Entity m_copied_entity;
	};

} // namespace ignis