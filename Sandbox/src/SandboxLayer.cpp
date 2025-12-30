#include "SandboxLayer.h"

SandBoxLayer::SandBoxLayer(ignis::Renderer& renderer)
	: Layer("SandboxLayer"), m_renderer(renderer)
{
}

void SandBoxLayer::OnAttach()
{
	m_shader_library = std::make_shared<ignis::ShaderLibrary>();
	m_shader_library->Load("assets://shaders/example.glsl");
	m_shader_library->Load("assets://shaders/blinn.glsl");

	m_camera = ignis::Camera(45.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
	m_camera.SetPosition({ 1.5f, 0.0f, 10.0f });
	m_camera.RecalculateViewMatrix();

	m_scene = ignis::Scene();

	auto face = m_scene.CreateEntity("Smiling Face");
	face.RemoveComponent<ignis::TagComponent>();
	ignis::Log::CoreInfo("Has TagComponent: {}", face.HasComponent<ignis::TagComponent>());

	ignis::AssetHandle mesh_handle = ignis::AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
	//ignis::AssetHandle mesh_handle = ignis::AssetManager::ImportAsset("assets://models/backpack/backpack.obj");
	m_mesh = ignis::AssetManager::GetAsset<ignis::Mesh>(mesh_handle);
	m_mesh->FlipUVs();
	auto normal_map_handle = ignis::AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga");
	m_mesh->SetMaterialDataTexture(0, ignis::MaterialType::Normal, normal_map_handle);
	auto metallic_map_handle = ignis::AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga");
	m_mesh->SetMaterialDataTexture(0, ignis::MaterialType::Metal, metallic_map_handle);
	auto roughness_map_handle = ignis::AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_R.tga");
	m_mesh->SetMaterialDataTexture(0, ignis::MaterialType::Roughness, roughness_map_handle);
	m_renderer.BeginScene();

	ignis::UUID test_id = ignis::UUID();
	ignis::Log::CoreInfo("Generated UUID: {}", test_id.ToString());
	ignis::Log::CoreInfo("Generated UUID is valid: {}", test_id.IsValid());
	test_id = ignis::UUID("Invalid ID");
	ignis::Log::CoreInfo("Generated UUID is valid: {}", test_id.IsValid());
	m_pipeline = std::make_shared<ignis::PBRPipeline>(m_shader_library);

	m_transform_component.Scale *= 0.1;
	m_transform_component.Rotation = glm::vec3(0, glm::radians(90.0f), glm::radians(90.0f));
}

void SandBoxLayer::OnUpdate(float dt)
{
	static glm::mat4 model = glm::mat4(1.0f);
	if (ignis::Input::IsKeyPressed(ignis::KeyCode::W))
	{
		glm::vec3 forward = m_camera.GetForwardDirection();
		m_camera.SetPosition(m_camera.GetPosition() + forward * dt * m_camera_speed);
		m_camera.RecalculateViewMatrix();
		auto camera_position = m_camera.GetPosition();
		ignis::Log::CoreInfo("Camera Position: {}, {}, {}", camera_position.x, camera_position.y, camera_position.z);
	}
	else if (ignis::Input::IsKeyPressed(ignis::KeyCode::S))
	{
		glm::vec3 forward = m_camera.GetForwardDirection();
		m_camera.SetPosition(m_camera.GetPosition() - forward * dt * m_camera_speed);
		m_camera.RecalculateViewMatrix();
		auto position = m_camera.GetPosition();
		ignis::Log::CoreInfo("Position {}, {}, {}", position.x, position.y, position.z);
	}
	else if (ignis::Input::IsKeyPressed(ignis::KeyCode::A))
	{
		glm::vec3 right = m_camera.GetRightDirection();
		m_camera.SetPosition(m_camera.GetPosition() - right * dt * m_camera_speed);
		m_camera.RecalculateViewMatrix();
		auto position = m_camera.GetPosition();
		ignis::Log::CoreInfo("Position {}, {}, {}", position.x, position.y, position.z);
	}
	else if (ignis::Input::IsKeyPressed(ignis::KeyCode::D))
	{
		glm::vec3 right = m_camera.GetRightDirection();
		m_camera.SetPosition(m_camera.GetPosition() + right * dt * m_camera_speed);
		m_camera.RecalculateViewMatrix();
		auto position = m_camera.GetPosition();
		ignis::Log::CoreInfo("Position {}, {}, {}", position.x, position.y, position.z);
	}

	m_renderer.Clear();

	m_renderer.RenderMesh(m_pipeline, m_camera, m_mesh, m_transform_component.GetTransform(), m_light_dir);
}

void SandBoxLayer::OnEvent(ignis::EventBase& event)
{
	if (auto* resize_event = dynamic_cast<ignis::WindowResizeEvent*>(&event))
	{
		float aspect_ratio = static_cast<float>(resize_event->GetWidth()) / static_cast<float>(resize_event->GetHeight());
		m_camera.SetPerspective(45.0f, aspect_ratio, 0.1f, 1000.0f);
		m_camera.RecalculateViewMatrix();
	}
}