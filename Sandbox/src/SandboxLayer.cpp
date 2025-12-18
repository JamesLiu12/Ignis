#include "SandboxLayer.h"

SandBoxLayer::SandBoxLayer(ignis::Renderer& renderer)
	: Layer("SandboxLayer"), m_renderer(renderer)
{
}

void SandBoxLayer::OnAttach()
{
	float vertices[] = {
		 0.5f,  0.5f, 0.0f,   1.0f, 1.0f,   // top right
		 0.5f, -0.5f, 0.0f,   1.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   // bottom left
		-0.5f,  0.5f, 0.0f,   0.0f, 1.0f    // top left 
	};

	uint32_t indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	m_vb = ignis::VertexBuffer::Create(vertices, sizeof(vertices));
	m_vb->SetLayout(ignis::VertexBuffer::Layout({
		{0, ignis::Shader::DataType::Float3, false, 0},
		{1, ignis::Shader::DataType::Float2, false, sizeof(float) * 3}
	}));

	m_ib = ignis::IndexBuffer::Create(indices, sizeof(indices));

	m_va = ignis::VertexArray::Create();
	m_va->AddVertexBuffer(m_vb);
	m_va->SetIndexBuffer(m_ib);
	m_va->UnBind();

	m_texture = ignis::Texture2D::CreateFromFile(
		ignis::TextureSpecs{
			.SourceFormat = ignis::ImageFormat::RGBA,
			.InternalFormat = ignis::ImageFormat::RGB,
		},
		"assets://images/awesomeface.png",
		true
	);

	m_shader_library = ignis::ShaderLibrary();
	m_shader_library.Load("assets://shaders/example.glsl");
	m_shader_library.Load("assets://shaders/blinn.glsl");

	m_camera = ignis::Camera(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
	m_camera.SetPosition({ 0.0f, 0.0f, 3.0f });
	m_camera.RecalculateViewMatrix();

	m_scene = ignis::Scene();

	auto face = m_scene.CreateEntity("Smiling Face");
	face.RemoveComponent<ignis::TagComponent>();
	ignis::Log::CoreInfo("Has TagComponent: {}", face.HasComponent<ignis::TagComponent>());

	ignis::AssetHandle mesh_handle = ignis::AssetManager::ImportAsset("assets://models/backpack/backpack.obj");
	m_mesh = ignis::AssetManager::GetAsset<ignis::Mesh>(mesh_handle);
	m_mesh = ignis::AssetManager::GetAsset<ignis::Mesh>(mesh_handle);
	m_renderer.BeginScene();

	ignis::UUID test_id = ignis::UUID();
	ignis::Log::CoreInfo("Generated UUID: {}", test_id.ToString());
	ignis::Log::CoreInfo("Generated UUID is valid: {}", test_id.IsValid());
	test_id = ignis::UUID("Invalid ID");
	ignis::Log::CoreInfo("Generated UUID is valid: {}", test_id.IsValid());
}

void SandBoxLayer::OnUpdate(float dt)
{
	static glm::mat4 model = glm::mat4(1.0f);
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

	//ignis::Shader& shader = m_shader_library.Get("assets://shaders/example.glsl");
	ignis::Shader& shader = m_shader_library.Get("assets://shaders/blinn.glsl");
	
	//m_texture->Bind(0);
	shader.Bind();
	//shader.Set("uViewProjection", m_camera.GetViewProjection());
	//shader.Set("uTexture", 0);
	shader.Set("view", m_camera.GetView());
	shader.Set("projection", m_camera.GetProjection());
	model = glm::rotate(model, glm::radians(-55.0f) * dt, glm::vec3(0.0f, 1.0f, 0.0f));
	shader.Set("model", model);
	shader.Set("viewPos", m_camera.GetPosition());
	shader.Set("dirLight.direction", glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f)));
	shader.Set("dirLight.ambient", glm::vec3(0.10f));
	shader.Set("dirLight.diffuse", glm::vec3(0.80f));
	shader.Set("dirLight.specular", glm::vec3(1.00f));
	m_va->Bind();
	//m_renderer.DrawIndexed(*m_va);
	m_renderer.RenderMesh(m_mesh, shader);
}

void SandBoxLayer::OnEvent(ignis::EventBase& event)
{
	if (auto* resize_event = dynamic_cast<ignis::WindowResizeEvent*>(&event))
	{
		float aspect_ratio = static_cast<float>(resize_event->GetWidth()) / static_cast<float>(resize_event->GetHeight());
		m_camera.SetPerspective(45.0f, aspect_ratio, 0.1f, 100.0f);
		m_camera.RecalculateViewMatrix();
	}
}