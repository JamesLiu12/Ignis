#pragma once

#include "Ignis.h"

// Forward declarations for editor-specific types
namespace ignis {
	class EditorLayer;
	class PropertiesPanel;
	class SceneHierarchyPanel;
}

class EditorApp : public ignis::Application
{
public:
	EditorApp();
	~EditorApp();

	// Editor-specific accessors
	ignis::PropertiesPanel* GetPropertiesPanel() { return m_properties_panel.get(); }
	ignis::SceneHierarchyPanel* GetSceneHierarchyPanel() { return m_scene_hierarchy_panel.get(); }

private:
	ignis::EditorLayer* m_editor_layer;
	std::shared_ptr<ignis::PropertiesPanel> m_properties_panel;
	std::shared_ptr<ignis::SceneHierarchyPanel> m_scene_hierarchy_panel;
};
