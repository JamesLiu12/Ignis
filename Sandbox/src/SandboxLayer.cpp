#include "SandboxLayer.h"

SandBoxLayer::SandBoxLayer(ignis::Renderer& renderer)
	: Layer("SandboxLayer"), m_renderer(renderer)
{
}

void SandBoxLayer::OnAttach()
{
	float vertices[] = {
		-0.5f, 0.5f, -1.0f,
		-0.5, -0.5f, -1.0f,
		0.5f, -0.5f, -1.0f,
		0.5f, 0.5f, -1.0f
	};

	uint32_t indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	m_vb = ignis::VertexBuffer::Create(vertices, sizeof(vertices));
	m_vb->SetLayout(ignis::VertexBuffer::Layout({ ignis::VertexBuffer::Attribute(0, ignis::Shader::DataType::Float3) }));

	m_ib = ignis::IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t));

	m_va = ignis::VertexArray::Create();
	m_va->AddVertexBuffer(m_vb);
	m_va->SetIndexBuffer(m_ib);

	std::string shader_path = "assets/shaders/example.glsl";

	m_shader = ignis::Shader::CreateFromFile(shader_path);

	m_camera = ignis::Camera(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
}

void SandBoxLayer::OnUpdate(float dt)
{
	if (ignis::Input::IsKeyPressed(ignis::KeyCode::W))
	{
		glm::vec3 forward = m_camera.GetForwardDirection();
		m_camera.SetPosition(m_camera.GetPosition() + forward * dt);
		m_camera.RecalculateViewMatrix();
		auto camera_position = m_camera.GetPosition();
		ignis::Log::CoreInfo("Camera Position: {}, {}, {}", camera_position.x, camera_position.y, camera_position.z);
	}
	else if (ignis::Input::IsKeyPressed(ignis::KeyCode::S))
	{
		glm::vec3 forward = m_camera.GetForwardDirection();
		m_camera.SetPosition(m_camera.GetPosition() - forward * dt);
		m_camera.RecalculateViewMatrix();
		auto position = m_camera.GetPosition();
		ignis::Log::CoreInfo("Position {}, {}, {}", position.x, position.y, position.z);
	}
	else if (ignis::Input::IsKeyPressed(ignis::KeyCode::A))
	{
		glm::vec3 right = m_camera.GetRightDirection();
		m_camera.SetPosition(m_camera.GetPosition() - right * dt);
		m_camera.RecalculateViewMatrix();
		auto position = m_camera.GetPosition();
		ignis::Log::CoreInfo("Position {}, {}, {}", position.x, position.y, position.z);
	}
	else if (ignis::Input::IsKeyPressed(ignis::KeyCode::D))
	{
		glm::vec3 right = m_camera.GetRightDirection();
		m_camera.SetPosition(m_camera.GetPosition() + right * dt);
		m_camera.RecalculateViewMatrix();
		auto position = m_camera.GetPosition();
		ignis::Log::CoreInfo("Position {}, {}, {}", position.x, position.y, position.z);
	}

	m_renderer.Clear();

	m_shader->Bind();
	m_shader->Set("uViewProjection", m_camera.GetViewProjection());
	m_va->Bind();
	m_renderer.DrawIndexed(*m_va);
}

void SandBoxLayer::OnEvent(ignis::EventBase& event)
{

}