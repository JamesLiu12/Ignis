#include "SandboxLayer.h"

SandBoxLayer::SandBoxLayer(ignis::Renderer& renderer)
	: Layer("SandboxLayer"), m_renderer(renderer)
{
}

void SandBoxLayer::OnAttach()
{
	m_camera = ignis::Camera(45.0f, 1280.0f / 720.0f, 0.1f, 100.0f);
	m_camera.SetPosition({ 1.5f, 0.0f, 8.0f });
	m_camera.RecalculateViewMatrix();

	m_scene = ignis::Scene();

	auto face = m_scene.CreateEntity("Smiling Face");
	face.RemoveComponent<ignis::TagComponent>();
	ignis::Log::CoreInfo("Has TagComponent: {}", face.HasComponent<ignis::TagComponent>());

	ignis::AssetHandle mesh_handle = ignis::AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
	//ignis::AssetHandle mesh_handle = ignis::AssetManager::ImportAsset("assets://models/backpack/backpack.obj");
	m_mesh = ignis::AssetManager::GetAsset<ignis::Mesh>(mesh_handle);
	m_mesh->FlipUVs();
	//auto normal_map_handle = ignis::AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga");
	//m_mesh->SetMaterialDataTexture(0, ignis::MaterialType::Normal, normal_map_handle);
	m_renderer.BeginScene();

	ignis::UUID test_id = ignis::UUID();
	ignis::Log::CoreInfo("Generated UUID: {}", test_id.ToString());
	ignis::Log::CoreInfo("Generated UUID is valid: {}", test_id.IsValid());
	test_id = ignis::UUID("Invalid ID");
	ignis::Log::CoreInfo("Generated UUID is valid: {}", test_id.IsValid());
	
	// Create a light entity for testing properties panel
	auto entity = m_scene.CreateEntity("Main Directional Light");
	m_light_entity = std::make_shared<ignis::Entity>(entity);
	
	auto& light = m_light_entity->AddComponent<ignis::DirectionalLightComponent>();
	light.Color = glm::vec3(1.0f, 0.95f, 0.8f); // Warm white light
	light.Intensity = 1.5f;
	light.Direction = glm::vec3(-0.2f, -1.0f, -0.3f);
	
	// Set light position (useful for point and spot lights)
	auto& lightTransform = m_light_entity->GetComponent<ignis::TransformComponent>();
	lightTransform.Translation = glm::vec3(0.0f, 5.0f, 5.0f);
	
	// Set this entity as selected in properties panel
	if (auto* properties_panel = ignis::Application::Get().GetPropertiesPanel())
	{
		properties_panel->SetSelectedEntity(m_light_entity);
		ignis::Log::CoreInfo("Light entity set as selected in properties panel");
	}
	else
	{
		ignis::Log::CoreWarn("Properties panel not found");
	}
	m_pipeline = std::make_shared<ignis::PBRPipeline>(m_shader_library);
}

void SandBoxLayer::OnUpdate(float dt)
{
	static glm::mat4 model = glm::mat4(1.0f);
	float camera_speed = 100.0f;
	if (ignis::Input::IsKeyPressed(ignis::KeyCode::W))
	{
		glm::vec3 forward = m_camera.GetForwardDirection();
		m_camera.SetPosition(m_camera.GetPosition() + forward * dt * camera_speed);
		m_camera.RecalculateViewMatrix();
		auto camera_position = m_camera.GetPosition();
		ignis::Log::CoreInfo("Camera Position: {}, {}, {}", camera_position.x, camera_position.y, camera_position.z);
	}
	else if (ignis::Input::IsKeyPressed(ignis::KeyCode::S))
	{
		glm::vec3 forward = m_camera.GetForwardDirection();
		m_camera.SetPosition(m_camera.GetPosition() - forward * dt * camera_speed);
		m_camera.RecalculateViewMatrix();
		auto position = m_camera.GetPosition();
		ignis::Log::CoreInfo("Position {}, {}, {}", position.x, position.y, position.z);
	}
	else if (ignis::Input::IsKeyPressed(ignis::KeyCode::A))
	{
		glm::vec3 right = m_camera.GetRightDirection();
		m_camera.SetPosition(m_camera.GetPosition() - right * dt * camera_speed);
		m_camera.RecalculateViewMatrix();
		auto position = m_camera.GetPosition();
		ignis::Log::CoreInfo("Position {}, {}, {}", position.x, position.y, position.z);
	}
	else if (ignis::Input::IsKeyPressed(ignis::KeyCode::D))
	{
		glm::vec3 right = m_camera.GetRightDirection();
		m_camera.SetPosition(m_camera.GetPosition() + right * dt * camera_speed);
		m_camera.RecalculateViewMatrix();
		auto position = m_camera.GetPosition();
		ignis::Log::CoreInfo("Position {}, {}, {}", position.x, position.y, position.z);
	}

	m_renderer.Clear();
	
	// Use light component data from the scene
	if (m_light_entity)
	{
		auto& transform = m_light_entity->GetComponent<ignis::TransformComponent>();
		
		// Check for Directional Light
		if (m_light_entity->HasComponent<ignis::DirectionalLightComponent>())
		{
			auto& light = m_light_entity->GetComponent<ignis::DirectionalLightComponent>();
			shader.Set("lightType", 0);
			shader.Set("dirLight.direction", glm::normalize(light.Direction));
			shader.Set("dirLight.ambient", light.Color * 0.1f);
			shader.Set("dirLight.diffuse", light.Color * light.Intensity);
			shader.Set("dirLight.specular", glm::vec3(1.0f));
		}
		// Check for Point Light
		else if (m_light_entity->HasComponent<ignis::PointLightComponent>())
		{
			auto& light = m_light_entity->GetComponent<ignis::PointLightComponent>();
			shader.Set("lightType", 1);
			shader.Set("pointLight.position", transform.Translation);
			shader.Set("pointLight.ambient", light.Color * 0.1f);
			shader.Set("pointLight.diffuse", light.Color * light.Intensity);
			shader.Set("pointLight.specular", glm::vec3(1.0f));
			shader.Set("pointLight.range", light.Range);
			shader.Set("pointLight.attenuation", light.Attenuation);
		}
		// Check for Spot Light
		else if (m_light_entity->HasComponent<ignis::SpotLightComponent>())
		{
			auto& light = m_light_entity->GetComponent<ignis::SpotLightComponent>();
			shader.Set("lightType", 2);
			shader.Set("spotLight.position", transform.Translation);
			shader.Set("spotLight.direction", glm::normalize(light.Direction));
			shader.Set("spotLight.ambient", light.Color * 0.1f);
			shader.Set("spotLight.diffuse", light.Color * light.Intensity);
			shader.Set("spotLight.specular", glm::vec3(1.0f));
			shader.Set("spotLight.range", light.Range);
			shader.Set("spotLight.attenuation", light.Attenuation);
			shader.Set("spotLight.innerConeAngle", light.InnerConeAngle);
			shader.Set("spotLight.outerConeAngle", light.OuterConeAngle);
		}
		else
		{
			// Fallback if entity has no light component
			shader.Set("lightType", 0);
			shader.Set("dirLight.direction", glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f)));
			shader.Set("dirLight.ambient", glm::vec3(0.10f));
			shader.Set("dirLight.diffuse", glm::vec3(0.80f));
			shader.Set("dirLight.specular", glm::vec3(1.0f));
		}
	}
	else
	{
		// Fallback to default directional light if no light entity
		shader.Set("lightType", 0);
		shader.Set("dirLight.direction", glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f)));
		shader.Set("dirLight.ambient", glm::vec3(0.10f));
		shader.Set("dirLight.diffuse", glm::vec3(0.80f));
		shader.Set("dirLight.specular", glm::vec3(1.0f));
	}

	m_renderer.RenderMesh(m_pipeline, m_camera, m_mesh);
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