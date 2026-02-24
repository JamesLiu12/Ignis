#include "Editor/EditorSceneLayer.h"
#include "Editor/EditorApp.h"
#include "Editor/Panels/PropertiesPanel.h"
#include "Editor/Panels/SceneHierarchyPanel.h"
#include <glm/gtc/matrix_transform.hpp>

EditorSceneLayer::EditorSceneLayer(ignis::Renderer& renderer, ignis::EditorApp* editor_app)
	: Layer("EditorSceneLayer"), m_renderer(renderer), m_editor_app(editor_app)
{
}

static void SceneHierarchyTest(ignis::Scene* scene)
{
	auto a = scene->CreateEntity();
	auto b = scene->CreateEntity();
	auto c = scene->CreateEntity();
	auto d = scene->CreateEntity();
	b.SetParent(a);
	c.SetParent(a);
	a.AddChild(d);

	std::unordered_map<ignis::UUID, std::string> id_to_name;
	id_to_name[a.GetID()] = "a";
	id_to_name[b.GetID()] = "b";
	id_to_name[c.GetID()] = "c";
	id_to_name[d.GetID()] = "d";

	ignis::Log::CoreInfo("{} is the parent of {}", id_to_name[b.GetParentID()], id_to_name[b.GetID()]);
	ignis::Log::CoreInfo("{} is the parent of {}", id_to_name[c.GetParentID()], id_to_name[c.GetID()]);
	ignis::Log::CoreInfo("{} is the parent of {}", id_to_name[d.GetParent().GetID()], id_to_name[d.GetID()]);

	auto children = a.GetChildren();
	for (const auto& child : children)
	{
		ignis::Log::CoreInfo("a's child: {}", id_to_name[child.GetID()]);
	}

	a.ForEachChild([&](ignis::Entity entity) {
		ignis::Log::CoreInfo("a's child: {}", id_to_name[entity.GetID()]);
		});

	b.Unparent();
	b.RemoveChild(c);
	b.AddChild(c);
	ignis::Log::CoreInfo("{} is the parent of {}", id_to_name[b.GetParentID()], id_to_name[b.GetID()]);
	ignis::Log::CoreInfo("{} is the parent of {}", id_to_name[c.GetParentID()], id_to_name[c.GetID()]);
	ignis::Log::CoreInfo("{} is the parent of {}", id_to_name[d.GetParent().GetID()], id_to_name[d.GetID()]);

	a.MoveToAfter(c);
	d.MoveToAfter(a);
	c.SetSiblingIndex(2);
	d.SetSiblingIndex(0);

	a.ForEachChild([&](ignis::Entity entity) {
		ignis::Log::CoreInfo("a's child: {}", id_to_name[entity.GetID()]);
		});
	b.ForEachChild([&](ignis::Entity entity) {
		ignis::Log::CoreInfo("b's child: {}", id_to_name[entity.GetID()]);
		});
}

void EditorSceneLayer::OnAttach()
{
	m_shader_library = std::make_shared<ignis::ShaderLibrary>();
	m_shader_library->Load("assets://shaders/example.glsl");
	m_shader_library->Load("assets://shaders/blinn.glsl");

	auto& window = m_editor_app->GetWindow();
	float aspect_ratio = static_cast<float>(window.GetFramebufferWidth()) / static_cast<float>(window.GetFramebufferHeight());
	m_camera = std::make_shared<ignis::EditorCamera>(45.0f, aspect_ratio, 0.1f, 1000.0f);
	m_camera->SetPosition({ 1.5f, 0.0f, 10.0f });
	m_camera->RecalculateViewMatrix();

	m_scene = std::make_shared<ignis::Scene>();

	auto face = m_scene->CreateEntity("Smiling Face");
	face.RemoveComponent<ignis::TagComponent>();
	ignis::Log::CoreInfo("Has TagComponent: {}", face.HasComponent<ignis::TagComponent>());

	ignis::AssetHandle mesh_handle = ignis::AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
	//ignis::AssetHandle mesh_handle = ignis::AssetManager::ImportAsset("assets://models/backpack/backpack.obj");
	//ignis::AssetHandle mesh_handle = ignis::AssetManager::ImportAsset("assets://models/sphere.fbx");
	m_mesh = ignis::AssetManager::GetAsset<ignis::Mesh>(mesh_handle);
	
	auto albedo_map_handle = ignis::AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_A.tga");
	m_mesh->SetMaterialDataTexture(0, ignis::MaterialType::Albedo, albedo_map_handle);
	auto normal_map_handle = ignis::AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga");
	m_mesh->SetMaterialDataTexture(0, ignis::MaterialType::Normal, normal_map_handle);
	auto metallic_map_handle = ignis::AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga");
	m_mesh->SetMaterialDataTexture(0, ignis::MaterialType::Metal, metallic_map_handle);
	auto roughness_map_handle = ignis::AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_R.tga");
	m_mesh->SetMaterialDataTexture(0, ignis::MaterialType::Roughness, roughness_map_handle);

	ignis::UUID test_id = ignis::UUID();
	ignis::Log::CoreInfo("Generated UUID: {}", test_id.ToString());
	ignis::Log::CoreInfo("Generated UUID is valid: {}", test_id.IsValid());
	test_id = ignis::UUID("Invalid ID");
	ignis::Log::CoreInfo("Generated UUID is valid: {}", test_id.IsValid());
	
	// Create Directional Light entity
	auto directional_light_entity = m_scene->CreateEntity("Directional Light");
	m_light_entity = std::make_shared<ignis::Entity>(directional_light_entity);

	auto& dir_light = m_light_entity->AddComponent<ignis::DirectionalLightComponent>();
	dir_light.Color = glm::vec3(1.0f, 0.95f, 0.8f); // Warm white light
	dir_light.Intensity = 1.5f;
	
	auto& dir_transform = m_light_entity->GetComponent<ignis::TransformComponent>();
	dir_transform.Translation = glm::vec3(0.0f, 5.0f, 5.0f);

	// Create Point Light entity
	auto point_light_entity = m_scene->CreateEntity("Point Light");
	auto& point_light = point_light_entity.AddComponent<ignis::PointLightComponent>();
	point_light.Color = glm::vec3(1.0f, 0.0f, 0.0f); // Red
	point_light.Intensity = 5.0f;
	point_light.Range = 10.0f;
	
	auto& point_transform = point_light_entity.GetComponent<ignis::TransformComponent>();
	point_transform.Translation = glm::vec3(2.0f, 2.0f, 0.0f);

	// Create Spot Light entity
	auto spot_light_entity = m_scene->CreateEntity("Spot Light");
	auto& spot_light = spot_light_entity.AddComponent<ignis::SpotLightComponent>();
	spot_light.Color = glm::vec3(0.0f, 1.0f, 0.0f); // Green
	spot_light.Intensity = 10.0f;
	spot_light.Range = 15.0f;
	spot_light.InnerConeAngle = 12.5f;
	spot_light.OuterConeAngle = 17.5f;
	
	auto& spot_transform = spot_light_entity.GetComponent<ignis::TransformComponent>();
	spot_transform.Translation = glm::vec3(-2.0f, 2.0f, 0.0f);

	// Create Sky Light entity
	auto sky_light_entity = m_scene->CreateEntity("Sky Light");
	auto& sky_light_component = sky_light_entity.AddComponent<ignis::SkyLightComponent>();
	sky_light_component.SceneEnvironment.SetIBLMaps({
		ignis::AssetManager::ImportAsset("assets://images/brown_photostudio_02_4k/brown_photostudio_02_4k_irradiance.hdr", ignis::AssetType::EnvironmentMap),
		ignis::AssetManager::ImportAsset("assets://images/brown_photostudio_02_4k/brown_photostudio_02_4k_radiance.hdr", ignis::AssetType::EnvironmentMap)
		});
	sky_light_component.SceneEnvironment.SetSkyboxMap({
		ignis::AssetManager::ImportAsset("assets://images/brown_photostudio_02_4k/brown_photostudio_02_4k_skybox.hdr", ignis::AssetType::EnvironmentMap)
		});

	// Set the scene in the hierarchy panel
	if (auto* hierarchy_panel = m_editor_app->GetSceneHierarchyPanel())
	{
		hierarchy_panel->SetScene(m_scene);
		ignis::Log::CoreInfo("Scene set in hierarchy panel");
	}
	else
	{
		ignis::Log::CoreWarn("Scene hierarchy panel not found");
	}
	
	// Set directional light as initially selected in properties panel
	if (auto* properties_panel = m_editor_app->GetPropertiesPanel())
	{
		properties_panel->SetSelectedEntity(m_light_entity);
		ignis::Log::CoreInfo("Directional light entity set as selected in properties panel");
	}
	else
	{
		ignis::Log::CoreWarn("Properties panel not found");
	}
	m_pipeline = std::make_shared<ignis::PBRPipeline>(m_shader_library);

	m_mesh_transform_component.Scale *= 0.1;
	m_mesh_transform_component.Rotation = glm::vec3(0, glm::radians(90.0f), glm::radians(90.0f));
	
	// Connect mesh to PropertiesPanel for editing
	// Pass address of m_mesh so PropertiesPanel can update the same mesh we render
	auto* properties_panel = m_editor_app->GetPropertiesPanel();
	if (properties_panel)
	{
		properties_panel->SetCurrentMesh(&m_mesh, &m_mesh_transform_component);
	}

	SceneHierarchyTest(m_scene.get());

	ignis::FrameBufferSpecs specs;
	specs.Width = window.GetFramebufferWidth();
	specs.Height = window.GetFramebufferHeight();
	specs.Attachments = { ignis::TextureFormat::RGBA8, ignis::TextureFormat::Depth24Stencil8 };

	auto framebuffer = ignis::Framebuffer::Create(specs);
	m_renderer.SetFramebuffer(framebuffer);
}

void EditorSceneLayer::OnUpdate(float dt)
{
	static glm::mat4 model = glm::mat4(1.0f);
	// Update editor camera with mouse and keyboard controls
	m_camera->OnUpdate(dt);

	m_renderer.BeginScene(m_pipeline, m_scene, m_camera);

	m_renderer.Clear();

	m_renderer.RenderMesh(m_mesh, m_mesh_transform_component.GetTransform());

	auto& window = m_editor_app->GetWindow();

	m_renderer.EndScene();
}

void EditorSceneLayer::OnEvent(ignis::EventBase& event)
{
	if (auto* resize_event = dynamic_cast<ignis::WindowResizeEvent*>(&event))
	{
		auto& window = m_editor_app->GetWindow();
		uint32_t fb_width = window.GetFramebufferWidth();
		uint32_t fb_height = window.GetFramebufferHeight();
		
		float aspect_ratio = static_cast<float>(fb_width) / static_cast<float>(fb_height);
		m_camera->SetPerspective(45.0f, aspect_ratio, 0.1f, 1000.0f);
		m_camera->RecalculateViewMatrix();

		auto framebuffer = m_renderer.GetFramebuffer();
		if (framebuffer)
		{
			framebuffer->Resize(fb_width, fb_height);
		}

		m_renderer.SetViewport(0, 0, fb_width, fb_height);
	}
}