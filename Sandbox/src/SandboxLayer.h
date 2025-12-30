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

private:
	ignis::Renderer& m_renderer;

	ignis::ShaderLibrary m_shader_library;

	std::shared_ptr<ignis::VertexArray> m_va;
	std::shared_ptr<ignis::VertexBuffer> m_vb;
	std::shared_ptr<ignis::IndexBuffer> m_ib;
	std::shared_ptr<ignis::Texture> m_texture;
	
	ignis::Camera m_camera;

	ignis::Scene m_scene;

	std::shared_ptr<ignis::Mesh> m_mesh;
	
	// Test light entity for properties panel (shared_ptr for weak_ptr compatibility)
	std::shared_ptr<ignis::Entity> m_light_entity;
};