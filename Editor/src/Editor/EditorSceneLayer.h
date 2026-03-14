#pragma once

#include "Ignis.h"

#include "Editor/Panels/EditorCamera.h"

#include "Ignis/Script/ScriptModule.h"
#include "Ignis/Asset/AssetSerializer.h"
#include "Ignis/Renderer/DebugRenderer.h" 
#include "Ignis/Renderer/EditorOverlayRenderer.h"

#include "Ignis/UI/UISystem.h"
#include "Ignis/UI/UIRenderer.h"

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

	~EditorSceneLayer() override;

	void OnAttach() override;
	void OnDetach() override;
	void OnUpdate(float dt) override;
	void OnEvent(EventBase& event) override;

	// Project lifecycle methods
	void ReloadProject();  // Called when project is loaded
	void ClearProject();   // Called when project is closed

	// Scene management
	void LoadScene(const std::filesystem::path& scene_path);  // Load a scene file into the editor
	void SaveCurrentScene();  // Save current scene to its file

	// Scene state transitions
	void OnScenePlay();
	void OnSceneStop();
	SceneState GetSceneState() const { return m_scene_state; }

	// Scene access
	std::shared_ptr<Scene> GetScene() const { return m_current_scene; }
	std::shared_ptr<Scene> GetEditorScene() const { return m_editor_scene; }
	std::filesystem::path GetCurrentScenePath() const { return m_current_scene_path; }

	// Mesh access for PropertiesPanel
	std::shared_ptr<Mesh> GetCurrentMesh() const { return m_mesh; }
	TransformComponent& GetMeshTransform() { return m_mesh_transform_component; }

	GizmoMode GetGizmoMode() const { return m_gizmo_mode; }
	void      SetGizmoMode(GizmoMode mode) { m_gizmo_mode = mode; }

	std::shared_ptr<EditorCamera> GetEditorCamera() const { return m_editor_camera; }
	Entity GetSelectedEntity() const;

	void OnScriptsReload();

private:
	void RenderEditorOverlay();

private:
	Renderer& m_renderer;
	EditorApp* m_editor_app;
	ViewportPanel* m_viewport_panel = nullptr;
	
	std::shared_ptr<EditorCamera> m_editor_camera;

	std::shared_ptr<Scene> m_editor_scene;   // Persistent scene for editing
	std::shared_ptr<Scene> m_runtime_scene;  // Temporary scene for play mode
	std::shared_ptr<Scene> m_current_scene;  // Points to active scene (editor or runtime)
	std::filesystem::path m_current_scene_path;  // Path to currently loaded scene file

	std::shared_ptr<Mesh> m_mesh;
	
	// Test light entity for properties panel (shared_ptr for weak_ptr compatibility)
	Entity m_light_entity;

	std::shared_ptr<Pipeline> m_pipeline;

	TransformComponent m_mesh_transform_component;

	std::unique_ptr<DebugRenderer>          m_debug_renderer;
	std::unique_ptr<EditorOverlayRenderer>  m_overlay_renderer;
	GizmoMode m_gizmo_mode = GizmoMode::Translate;

	float m_camera_speed = 10.0f;
	bool  m_started_camera_drag_in_viewport = false;

	SceneState   m_scene_state = SceneState::Edit;
	ScriptModule m_script_module;

	UISystem m_ui_system;
	UIRenderer m_ui_renderer{ Application::Get().GetRenderer() };

	bool m_right_pressed = false;
	bool m_is_locked = false;
	bool m_is_visible = true;
	bool m_is_in_scene = false;
};

} // namespace ignis