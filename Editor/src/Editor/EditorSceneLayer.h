#pragma once

#include "Ignis.h"
#include "Editor/Panels/EditorCamera.h"

namespace ignis {
	class Renderer;
	class EditorApp;
	class ViewportPanel;
}

class EditorSceneLayer : public ignis::Layer
{
public:
	EditorSceneLayer(ignis::Renderer& renderer, ignis::EditorApp* editor_app);

	~EditorSceneLayer() override = default;

	void OnAttach() override;
	void OnUpdate(float dt) override;
	void OnEvent(ignis::EventBase& event) override;

	// Mesh access for PropertiesPanel
	std::shared_ptr<ignis::Mesh> GetCurrentMesh() const { return m_mesh; }
	ignis::TransformComponent& GetMeshTransform() { return m_mesh_transform_component; }

private:
	ignis::Renderer& m_renderer;
	ignis::EditorApp* m_editor_app;
	ignis::ViewportPanel* m_viewport_panel = nullptr;

	std::shared_ptr<ignis::ShaderLibrary> m_shader_library;
	
	std::shared_ptr<ignis::EditorCamera> m_camera;

	std::shared_ptr<ignis::Scene> m_scene;

	std::shared_ptr<ignis::Mesh> m_mesh;
	
	// Test light entity for properties panel (shared_ptr for weak_ptr compatibility)
	std::shared_ptr<ignis::Entity> m_light_entity;

	std::shared_ptr<ignis::Pipeline> m_pipeline;

	ignis::TransformComponent m_mesh_transform_component;

	float m_camera_speed = 10.0f;
};