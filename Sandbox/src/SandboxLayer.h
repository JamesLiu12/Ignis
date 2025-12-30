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

	std::shared_ptr<ignis::ShaderLibrary> m_shader_library;
	
	ignis::Camera m_camera;

	ignis::Scene m_scene;

	std::shared_ptr<ignis::Mesh> m_mesh;

	std::shared_ptr<ignis::Pipeline> m_pipeline;

	ignis::TransformComponent m_transform_component;

	float m_camera_speed = 10.0f;
};