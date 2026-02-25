#pragma once

#include "Ignis.h"
#include "Editor/Panels/EditorCamera.h"

namespace ignis {
	class Renderer;
	class EditorApp;
	class ViewportPanel;

class EditorSceneLayer : public Layer
{
public:
	EditorSceneLayer(Renderer& renderer, EditorApp* editor_app);

	~EditorSceneLayer() override = default;

	void OnAttach() override;
	void OnUpdate(float dt) override;
	void OnEvent(EventBase& event) override;

	// Mesh access for PropertiesPanel
	std::shared_ptr<Mesh> GetCurrentMesh() const { return m_mesh; }
	TransformComponent& GetMeshTransform() { return m_mesh_transform_component; }

private:
	Renderer& m_renderer;
	EditorApp* m_editor_app;
	ViewportPanel* m_viewport_panel = nullptr;

	std::shared_ptr<ShaderLibrary> m_shader_library;
	
	std::shared_ptr<EditorCamera> m_camera;

	std::shared_ptr<Scene> m_scene;

	std::shared_ptr<Mesh> m_mesh;
	
	// Test light entity for properties panel (shared_ptr for weak_ptr compatibility)
	Entity m_light_entity;

	std::shared_ptr<Pipeline> m_pipeline;

	TransformComponent m_mesh_transform_component;

	float m_camera_speed = 10.0f;
	
	// Camera input gate for tracking if drag started in viewport
	bool m_started_camera_drag_in_viewport = false;
};

} // namespace ignis