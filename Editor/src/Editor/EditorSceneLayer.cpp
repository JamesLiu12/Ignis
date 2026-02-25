#include "Editor/EditorSceneLayer.h"
#include "Editor/EditorApp.h"
#include "Editor/Panels/PropertiesPanel.h"
#include "Editor/Panels/SceneHierarchyPanel.h"
#include "Editor/Panels/ViewportPanel.h"
#include <glm/gtc/matrix_transform.hpp>

namespace ignis {

EditorSceneLayer::EditorSceneLayer(Renderer& renderer, EditorApp* editor_app)
	: Layer("EditorSceneLayer"), m_renderer(renderer), m_editor_app(editor_app)
{
}

static void SceneHierarchyTest(Scene* scene)
{
	auto a = scene->CreateEntity();
	auto b = scene->CreateEntity();
	auto c = scene->CreateEntity();
	auto d = scene->CreateEntity();
	b.SetParent(a);
	c.SetParent(a);
	a.AddChild(d);

	std::unordered_map<UUID, std::string> id_to_name;
	id_to_name[a.GetID()] = "a";
	id_to_name[b.GetID()] = "b";
	id_to_name[c.GetID()] = "c";
	id_to_name[d.GetID()] = "d";

	Log::CoreInfo("{} is the parent of {}", id_to_name[b.GetParentID()], id_to_name[b.GetID()]);
	Log::CoreInfo("{} is the parent of {}", id_to_name[c.GetParentID()], id_to_name[c.GetID()]);
	Log::CoreInfo("{} is the parent of {}", id_to_name[d.GetParent().GetID()], id_to_name[d.GetID()]);

	auto children = a.GetChildren();
	for (const auto& child : children)
	{
		Log::CoreInfo("a's child: {}", id_to_name[child.GetID()]);
	}

	a.ForEachChild([&](Entity entity) {
		Log::CoreInfo("a's child: {}", id_to_name[entity.GetID()]);
		});

	b.Unparent();
	b.RemoveChild(c);
	b.AddChild(c);
	Log::CoreInfo("{} is the parent of {}", id_to_name[b.GetParentID()], id_to_name[b.GetID()]);
	Log::CoreInfo("{} is the parent of {}", id_to_name[c.GetParentID()], id_to_name[c.GetID()]);
	Log::CoreInfo("{} is the parent of {}", id_to_name[d.GetParent().GetID()], id_to_name[d.GetID()]);

	a.MoveToAfter(c);
	d.MoveToAfter(a);
	c.SetSiblingIndex(2);
	d.SetSiblingIndex(0);

	a.ForEachChild([&](Entity entity) {
		Log::CoreInfo("a's child: {}", id_to_name[entity.GetID()]);
		});
	b.ForEachChild([&](Entity entity) {
		Log::CoreInfo("b's child: {}", id_to_name[entity.GetID()]);
		});
}

void EditorSceneLayer::OnAttach()
{
	m_shader_library = std::make_shared<ShaderLibrary>();
	m_shader_library->Load("resources://shaders/example.glsl");
	m_shader_library->Load("resources://shaders/blinn.glsl");

	auto& window = m_editor_app->GetWindow();
	float aspect_ratio = static_cast<float>(window.GetFramebufferWidth()) / static_cast<float>(window.GetFramebufferHeight());
	m_camera = std::make_shared<EditorCamera>(45.0f, aspect_ratio, 0.1f, 1000.0f);
	m_camera->SetPosition({ 1.5f, 0.0f, 10.0f });
	m_camera->RecalculateViewMatrix();

	m_scene = std::make_shared<Scene>();

	auto face = m_scene->CreateEntity("Smiling Face");
	face.RemoveComponent<TagComponent>();
	Log::CoreInfo("Has TagComponent: {}", face.HasComponent<TagComponent>());

	AssetHandle mesh_handle = AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
	//AssetHandle mesh_handle = AssetManager::ImportAsset("assets://models/backpack/backpack.obj");
	//AssetHandle mesh_handle = AssetManager::ImportAsset("assets://models/sphere.fbx");
	m_mesh = AssetManager::GetAsset<Mesh>(mesh_handle);
	
	auto albedo_map_handle = AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_A.tga");
	m_mesh->SetMaterialDataTexture(0, MaterialType::Albedo, albedo_map_handle);
	auto normal_map_handle = AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga");
	m_mesh->SetMaterialDataTexture(0, MaterialType::Normal, normal_map_handle);
	auto metallic_map_handle = AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga");
	m_mesh->SetMaterialDataTexture(0, MaterialType::Metal, metallic_map_handle);
	auto roughness_map_handle = AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_R.tga");
	m_mesh->SetMaterialDataTexture(0, MaterialType::Roughness, roughness_map_handle);

	UUID test_id = UUID();
	Log::CoreInfo("Generated UUID: {}", test_id.ToString());
	Log::CoreInfo("Generated UUID is valid: {}", test_id.IsValid());
	test_id = UUID("Invalid ID");
	Log::CoreInfo("Generated UUID is valid: {}", test_id.IsValid());
	
	// Create Directional Light entity
	auto directional_light_entity = m_scene->CreateEntity("Directional Light");
	m_light_entity = std::make_shared<Entity>(directional_light_entity);

	auto& dir_light = m_light_entity->AddComponent<DirectionalLightComponent>();
	dir_light.Color = glm::vec3(1.0f, 0.95f, 0.8f); // Warm white light
	dir_light.Intensity = 1.5f;
	
	auto& dir_transform = m_light_entity->GetComponent<TransformComponent>();
	dir_transform.Translation = glm::vec3(0.0f, 5.0f, 5.0f);

	// Create Point Light entity
	auto point_light_entity = m_scene->CreateEntity("Point Light");
	auto& point_light = point_light_entity.AddComponent<PointLightComponent>();
	point_light.Color = glm::vec3(1.0f, 0.0f, 0.0f); // Red
	point_light.Intensity = 5.0f;
	point_light.Range = 10.0f;
	
	auto& point_transform = point_light_entity.GetComponent<TransformComponent>();
	point_transform.Translation = glm::vec3(2.0f, 2.0f, 0.0f);

	// Create Spot Light entity
	auto spot_light_entity = m_scene->CreateEntity("Spot Light");
	auto& spot_light = spot_light_entity.AddComponent<SpotLightComponent>();
	spot_light.Color = glm::vec3(0.0f, 1.0f, 0.0f); // Green
	spot_light.Intensity = 10.0f;
	spot_light.Range = 15.0f;
	spot_light.InnerConeAngle = 12.5f;
	spot_light.OuterConeAngle = 17.5f;
	
	auto& spot_transform = spot_light_entity.GetComponent<TransformComponent>();
	spot_transform.Translation = glm::vec3(-2.0f, 2.0f, 0.0f);

	// Create Sky Light entity
	auto sky_light_entity = m_scene->CreateEntity("Sky Light");
	auto& sky_light_component = sky_light_entity.AddComponent<SkyLightComponent>();
	sky_light_component.SceneEnvironment.SetIBLMaps({
		AssetManager::ImportAsset("assets://images/brown_photostudio_02_4k/brown_photostudio_02_4k_irradiance.hdr", AssetType::EnvironmentMap),
		AssetManager::ImportAsset("assets://images/brown_photostudio_02_4k/brown_photostudio_02_4k_radiance.hdr", AssetType::EnvironmentMap)
		});
	sky_light_component.SceneEnvironment.SetSkyboxMap({
		AssetManager::ImportAsset("assets://images/brown_photostudio_02_4k/brown_photostudio_02_4k_skybox.hdr", AssetType::EnvironmentMap)
		});

	// Set the scene in the hierarchy panel
	if (auto* hierarchy_panel = m_editor_app->GetSceneHierarchyPanel())
	{
		hierarchy_panel->SetScene(m_scene);
		Log::CoreInfo("Scene set in hierarchy panel");
	}
	else
	{
		Log::CoreWarn("Scene hierarchy panel not found");
	}
	
	// Set directional light as initially selected in properties panel
	if (auto* properties_panel = m_editor_app->GetPropertiesPanel())
	{
		properties_panel->SetSelectedEntity(m_light_entity);
		Log::CoreInfo("Directional light entity set as selected in properties panel");
	}
	else
	{
		Log::CoreWarn("Properties panel not found");
	}
	m_pipeline = std::make_shared<PBRPipeline>(m_shader_library);

	m_mesh_transform_component.Scale *= 0.1;
	m_mesh_transform_component.Rotation = glm::vec3(0, glm::radians(90.0f), glm::radians(90.0f));
	
	// Connect mesh to PropertiesPanel for editing
	// Pass address of m_mesh so PropertiesPanel can update the same mesh we render
	auto* properties_panel = m_editor_app->GetPropertiesPanel();
	if (properties_panel)
	{
		properties_panel->SetCurrentMesh(&m_mesh, &m_mesh_transform_component);
	}

	// Get viewport panel reference for camera aspect ratio updates
	m_viewport_panel = m_editor_app->GetViewportPanel();

	SceneHierarchyTest(m_scene.get());

	FrameBufferSpecs specs;
	specs.Width = window.GetFramebufferWidth();
	specs.Height = window.GetFramebufferHeight();
	specs.Attachments = { TextureFormat::RGBA8, TextureFormat::Depth24Stencil8 };

	auto framebuffer = Framebuffer::Create(specs);
	m_renderer.SetFramebuffer(framebuffer);

	SceneSerializer scene_serializer;
	scene_serializer.Serialize(*m_scene, "MyProject/TestScene.scene");
	auto saved_scene = scene_serializer.Deserialize("MyProject/TestScene.scene");
	scene_serializer.Serialize(*saved_scene, "MyProject/TestScene.tscene");
}

void EditorSceneLayer::OnUpdate(float dt)
{
	static glm::mat4 model = glm::mat4(1.0f);
	
	// Camera input gating: only allow camera control when mouse is over viewport
	bool allow_camera_control = false;
	
	// Lazy initialization in which viewportPanel is created after EditorSceneLayer
	if (!m_viewport_panel)
	{
		m_viewport_panel = m_editor_app->GetViewportPanel();
	}
	
	if (m_viewport_panel)
	{
		// Get viewport bounds
		ImVec2 min_bound = m_viewport_panel->GetViewportMinBound();
		ImVec2 max_bound = m_viewport_panel->GetViewportMaxBound();
		
		// Check if mouse is within viewport bounds (use ImGui coordinates)
		ImVec2 mouse_pos = ImGui::GetMousePos();
		bool mouse_in_viewport = (mouse_pos.x >= min_bound.x && mouse_pos.x <= max_bound.x &&
		                          mouse_pos.y >= min_bound.y && mouse_pos.y <= max_bound.y);
		
		bool viewport_focused = m_viewport_panel->IsFocused();
		
		// Track camera drag state
		bool camera_button_pressed = Input::IsMouseButtonPressed(MouseButton::Left) || 
		                             Input::IsMouseButtonPressed(MouseButton::Right);
		
		// Start drag if mouse pressed in focused viewport
		if (camera_button_pressed && !m_started_camera_drag_in_viewport && 
		    viewport_focused && mouse_in_viewport)
		{
			m_started_camera_drag_in_viewport = true;
		}
		
		// Clear drag flag when mouse released
		if (!camera_button_pressed)
		{
			m_started_camera_drag_in_viewport = false;
		}
		
		// Allow camera control if mouse in viewport OR drag started in viewport
		allow_camera_control = (mouse_in_viewport && viewport_focused) || 
		                       m_started_camera_drag_in_viewport;
	}
	
	// Only update camera if allowed
	if (allow_camera_control)
	{
		m_camera->OnUpdate(dt);
	}

	// Update camera aspect ratio based on viewport panel size
	if (m_viewport_panel)
	{
		ImVec2 viewport_size = m_viewport_panel->GetViewportSize();
		if (viewport_size.x > 0 && viewport_size.y > 0)
		{
			float aspect = viewport_size.x / viewport_size.y;
			m_camera->SetPerspective(45.0f, aspect, 0.1f, 1000.0f);
		}
	}

	m_renderer.BeginScene(m_pipeline, m_scene, m_camera);

	m_renderer.Clear();

	m_renderer.RenderMesh(m_mesh, m_mesh_transform_component.GetTransform());

	auto& window = m_editor_app->GetWindow();

	m_renderer.EndScene();
}

void EditorSceneLayer::OnEvent(EventBase& event)
{
	// window resize handling removed, and viewport panel now manages framebuffer size
	// and camera aspect ratio is updated in OnUpdate() based on viewport panel size
}

} // namespace ignis