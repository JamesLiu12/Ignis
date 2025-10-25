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
	
	ignis::Camera m_camera;

};