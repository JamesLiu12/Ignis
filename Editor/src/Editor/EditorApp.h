#pragma once

#include "Ignis/Physics/PhysicsWorld.h"
#include "Ignis/Core/Application.h"

namespace ignis {

// Forward declarations
class EditorLayer;
class EditorSceneLayer;
class PropertiesPanel;
class SceneHierarchyPanel;
class ViewportPanel;
class AssetBrowserPanel;

class EditorApp : public Application
{
public:
	EditorApp();
	~EditorApp() override;

	void OnUpdate(float dt) override;

	// Editor-specific accessors
	PropertiesPanel* GetPropertiesPanel() { return m_properties_panel.get(); }
	SceneHierarchyPanel* GetSceneHierarchyPanel() { return m_scene_hierarchy_panel.get(); }
	ViewportPanel* GetViewportPanel() { return m_viewport_panel.get(); }
	AssetBrowserPanel* GetAssetBrowserPanel() { return m_asset_browser_panel.get(); }
	EditorSceneLayer* GetSceneLayer() { return m_scene_layer; }

private:
	void CreatePhysicsTestScene(); // Test scene for physics debugging
	
	EditorLayer* m_editor_layer = nullptr;
	EditorSceneLayer* m_scene_layer = nullptr;
	std::shared_ptr<PropertiesPanel> m_properties_panel;
	std::shared_ptr<SceneHierarchyPanel> m_scene_hierarchy_panel;
	std::shared_ptr<ViewportPanel> m_viewport_panel;
	std::shared_ptr<AssetBrowserPanel> m_asset_browser_panel;
	
	// Physics system for editor (with test scene)
	std::unique_ptr<PhysicsWorld> m_physics_world;
};

} // namespace ignis
