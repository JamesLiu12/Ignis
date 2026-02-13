#pragma once

#include "Ignis.h"

class SandBoxLayer;

namespace ignis {

// Forward declarations
class EditorLayer;
class PropertiesPanel;
class SceneHierarchyPanel;

class EditorApp : public Application
{
public:
	EditorApp();
	~EditorApp() override;

	// Editor-specific accessors
	PropertiesPanel* GetPropertiesPanel() { return m_properties_panel.get(); }
	SceneHierarchyPanel* GetSceneHierarchyPanel() { return m_scene_hierarchy_panel.get(); }

private:
	void CreatePhysicsTestScene(); // Test scene for physics debugging
	
	EditorLayer* m_editor_layer = nullptr;
	std::shared_ptr<PropertiesPanel> m_properties_panel;
	std::shared_ptr<SceneHierarchyPanel> m_scene_hierarchy_panel;
	
	// Physics system for editor (with test scene)
	std::unique_ptr<PhysicsWorld> m_physics_world;
};

} // namespace ignis
