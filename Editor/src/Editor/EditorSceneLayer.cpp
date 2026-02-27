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

void EditorSceneLayer::OnAttach()
{
	auto& window = m_editor_app->GetWindow();
	float aspect_ratio = static_cast<float>(window.GetFramebufferWidth()) / static_cast<float>(window.GetFramebufferHeight());
	m_camera = std::make_shared<EditorCamera>(45.0f, aspect_ratio, 0.1f, 1000.0f);
	m_camera->SetPosition({ 1.5f, 0.0f, 10.0f });
	m_camera->RecalculateViewMatrix();

	// Create pipeline and framebuffer (always needed)
	m_pipeline = std::make_shared<PBRPipeline>(m_renderer.GetShaderLibrary());
	
	FrameBufferSpecs specs;
	specs.Width = window.GetFramebufferWidth();
	specs.Height = window.GetFramebufferHeight();
	specs.Attachments = { TextureFormat::RGBA8, TextureFormat::Depth24Stencil8 };
	auto framebuffer = Framebuffer::Create(specs);
	m_renderer.SetFramebuffer(framebuffer);
	
	// Get viewport panel reference for camera aspect ratio updates
	m_viewport_panel = m_editor_app->GetViewportPanel();

	// Check if project is active before loading scene
	if (!Project::GetActive())
	{
		Log::CoreInfo("No active project - EditorSceneLayer starting in empty state");
		m_scene = nullptr;
		m_mesh = nullptr;
		return;
	}

	// Project is loaded - proceed with normal initialization
	AssetManager::LoadAssetRegistry(Project::GetActiveAssetRegistry());

	SceneSerializer scene_serializer;
	m_scene = scene_serializer.Deserialize(Project::GetActiveStartScene());

	AssetHandle mesh_handle = AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Cerberus_LP.FBX");
	m_mesh = AssetManager::GetAsset<Mesh>(mesh_handle);
	
	auto albedo_map_handle = AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_A.tga");
	m_mesh->SetMaterialDataTexture(0, MaterialType::Albedo, albedo_map_handle);
	auto normal_map_handle = AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_N.tga");
	m_mesh->SetMaterialDataTexture(0, MaterialType::Normal, normal_map_handle);
	auto metallic_map_handle = AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_M.tga");
	m_mesh->SetMaterialDataTexture(0, MaterialType::Metal, metallic_map_handle);
	auto roughness_map_handle = AssetManager::ImportAsset("assets://models/Cerberus_by_Andrew_Maximov/Textures/Cerberus_R.tga");
	m_mesh->SetMaterialDataTexture(0, MaterialType::Roughness, roughness_map_handle);

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
		properties_panel->SetSelectedEntity(std::make_shared<Entity>(m_light_entity));
		Log::CoreInfo("Directional light entity set as selected in properties panel");
	}
	else
	{
		Log::CoreWarn("Properties panel not found");
	}

	m_mesh_transform_component.Scale *= 0.1;
	m_mesh_transform_component.Rotation = glm::vec3(0, glm::radians(90.0f), glm::radians(90.0f));
	
	// Connect mesh to PropertiesPanel for editing
	auto* properties_panel = m_editor_app->GetPropertiesPanel();
	if (properties_panel)
	{
		properties_panel->SetCurrentMesh(&m_mesh, &m_mesh_transform_component);
	}

	SceneSerializer().Serialize(*m_scene, Project::GetActiveStartScene().replace_filename("StartSceneSaved.igscene"));
	AssetSerializer().Serialize(AssetManager::GetAssetRegistry(), Project::GetActiveAssetRegistry().replace_filename("TestARSaved.igar"));
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

	SceneRenderer scene_renderer(m_renderer);
	if (!m_scene)
	{
		auto framebuffer = m_renderer.GetFramebuffer();
		if (framebuffer)
		{
			framebuffer->Bind();
			m_renderer.Clear();
			framebuffer->UnBind();
		}
		return;
	}
	else
	{
		scene_renderer.BeginScene({ m_scene, m_camera, m_pipeline });
		m_scene->OnRender(scene_renderer);
		scene_renderer.EndScene();
	}
	
	auto& window = m_editor_app->GetWindow();
}

void EditorSceneLayer::OnEvent(EventBase& event)
{
	// window resize handling removed, and viewport panel now manages framebuffer size
	// and camera aspect ratio is updated in OnUpdate() based on viewport panel size
}

void EditorSceneLayer::ReloadProject()
{
	if (!Project::GetActive())
	{
		ClearProject();
		return;
	}
	
	Log::CoreInfo("Reloading project scene...");
	
	// Clear panels Before destroying old scene to prevent accessing stale entities
	if (auto* properties_panel = m_editor_app->GetPropertiesPanel())
	{
		properties_panel->SetSelectedEntity(nullptr);
		properties_panel->SetCurrentMesh(nullptr, nullptr);
	}
	
	// Clear previous project's scene and assets
	m_scene = nullptr;
	m_mesh = nullptr;
	
	// Reload asset registry and scene
	AssetManager::LoadAssetRegistry(Project::GetActiveAssetRegistry());
	SceneSerializer scene_serializer;
	m_scene = scene_serializer.Deserialize(Project::GetActiveStartScene());
	
	// Update hierarchy panel with all entities from the scene
	if (auto* hierarchy_panel = m_editor_app->GetSceneHierarchyPanel())
	{
		hierarchy_panel->SetScene(m_scene);
	}
	
	Log::CoreInfo("Project scene reloaded");
}

void EditorSceneLayer::ClearProject()
{
	m_scene = nullptr;
	m_mesh = nullptr;
	
	// Clear panels
	if (auto* hierarchy_panel = m_editor_app->GetSceneHierarchyPanel())
	{
		hierarchy_panel->SetScene(nullptr);
	}
	
	if (auto* properties_panel = m_editor_app->GetPropertiesPanel())
	{
		properties_panel->SetSelectedEntity(nullptr);
		properties_panel->SetCurrentMesh(nullptr, nullptr);
	}
	
	Log::CoreInfo("Project scene cleared");
}

} // namespace ignis