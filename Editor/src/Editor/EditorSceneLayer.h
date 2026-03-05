#pragma once

#include "Ignis.h"

#include "Editor/Panels/EditorCamera.h"

#include "Ignis/Script/ScriptModule.h"
#include "Ignis/Asset/AssetSerializer.h"

namespace ignis {
	class Renderer;
	class EditorApp;
	class ViewportPanel;

class EditorSceneLayer : public Layer
{
public:
	enum class SceneState
	{
		Edit = 0,
		Play = 1
	};

	EditorSceneLayer(Renderer& renderer, EditorApp* editor_app);

	~EditorSceneLayer() override = default;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float dt) override;
	void OnEvent(EventBase& event) override;

	// Project lifecycle methods
	void ReloadProject();  // Called when project is loaded
	void ClearProject();   // Called when project is closed

	// Scene state transitions
	void OnScenePlay();
	void OnSceneStop();
	SceneState GetSceneState() const { return m_scene_state; }

	// Scene access
	std::shared_ptr<Scene> GetScene() const { return m_current_scene; }

	// Mesh access for PropertiesPanel
	std::shared_ptr<Mesh> GetCurrentMesh() const { return m_mesh; }
	TransformComponent& GetMeshTransform() { return m_mesh_transform_component; }

private:
	Renderer& m_renderer;
	EditorApp* m_editor_app;
	ViewportPanel* m_viewport_panel = nullptr;
	
	std::shared_ptr<EditorCamera> m_editor_camera;

	std::shared_ptr<Scene> m_editor_scene;   // Persistent scene for editing
	std::shared_ptr<Scene> m_runtime_scene;  // Temporary scene for play mode
	std::shared_ptr<Scene> m_current_scene;  // Points to active scene (editor or runtime)

	std::shared_ptr<Mesh> m_mesh;
	
	// Test light entity for properties panel (shared_ptr for weak_ptr compatibility)
	Entity m_light_entity;

	std::shared_ptr<Pipeline> m_pipeline;

	TransformComponent m_mesh_transform_component;

	float m_camera_speed = 10.0f;
	
	// Camera input gate for tracking if drag started in viewport
	bool m_started_camera_drag_in_viewport = false;

	SceneState m_scene_state = SceneState::Edit;

	ScriptModule m_script_module;
};

} // namespace ignis