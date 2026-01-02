#pragma once

#include "Ignis.h"

class SandBoxLayer : public ignis::Layer
{
public:
	SandBoxLayer(ignis::Renderer& renderer);

	~SandBoxLayer() override = default;

	void OnAttach() override;
	void OnUpdate(float dt) override;
	void OnEvent(ignis::EventBase& event) override;

	// Mesh access for PropertiesPanel
	std::shared_ptr<ignis::Mesh> GetCurrentMesh() const { return m_mesh; }
	ignis::TransformComponent& GetMeshTransform() { return m_mesh_transform_component; }

private:
	ignis::Renderer& m_renderer;

	std::shared_ptr<ignis::ShaderLibrary> m_shader_library;
	
	ignis::EditorCamera m_camera;

	ignis::Scene m_scene;

	std::shared_ptr<ignis::Mesh> m_mesh;
	
	// Test light entity for properties panel (shared_ptr for weak_ptr compatibility)
	std::shared_ptr<ignis::Entity> m_light_entity;

	std::shared_ptr<ignis::Pipeline> m_pipeline;

	ignis::TransformComponent m_mesh_transform_component;

	float m_camera_speed = 10.0f;
};