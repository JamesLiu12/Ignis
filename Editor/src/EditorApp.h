#pragma once

#include "Ignis.h"

namespace ignis {

// Forward declarations for editor-specific types
class EditorLayer;
class PropertiesPanel;
class SceneHierarchyPanel;

class EditorApp : public Application
{
public:
	EditorApp();
	~EditorApp();

	// Editor-specific accessors
	PropertiesPanel* GetPropertiesPanel() { return m_properties_panel.get(); }
	SceneHierarchyPanel* GetSceneHierarchyPanel() { return m_scene_hierarchy_panel.get(); }

private:
	EditorLayer* m_editor_layer;
	std::shared_ptr<PropertiesPanel> m_properties_panel;
	std::shared_ptr<SceneHierarchyPanel> m_scene_hierarchy_panel;
};

} // namespace ignis
