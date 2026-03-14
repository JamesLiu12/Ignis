#include "Editor/EditorSceneLayer.h"
#include "Editor/EditorApp.h"
#include "Editor/Panels/PropertiesPanel.h"
#include "Editor/Panels/SceneHierarchyPanel.h"
#include "Editor/Panels/ViewportPanel.h"
#include "Editor/Panels/AssetBrowserPanel.h"
#include "Ignis/Renderer/IBLBaker.h"
#include "Ignis/Core/Events/MouseEvents.h"
#include "Ignis/Core/Events/KeyEvents.h"
#include "Ignis/Audio/AudioEngine.h"

#include <glm/gtc/matrix_transform.hpp>
#include <ImGuizmo.h>

namespace ignis {

EditorSceneLayer::EditorSceneLayer(Renderer& renderer, EditorApp* editor_app)
	: Layer("EditorSceneLayer"), m_renderer(renderer), m_editor_app(editor_app)
{
}

EditorSceneLayer::~EditorSceneLayer()
{
	// Ensure we're in Edit mode before destruction
	if (m_scene_state == SceneState::Play && m_runtime_scene)
	{
		// Stop runtime and clear scripts before scene destruction
		// Note: Don't log here as logger already be destroyed during shutdown
		m_runtime_scene->OnRuntimeStop();
		m_script_module.UnregisterAll(ScriptRegistry::Get());
		m_script_module.Unload();
		m_runtime_scene = nullptr;
		
		m_scene_state = SceneState::Edit;
	}
}

void EditorSceneLayer::OnAttach()
{
	m_renderer.Init();
	AudioEngine::Get().Init();
	
	AssetManager::SetLoadContext({
		.IBLBakerService = IBLBaker::Create(m_renderer),
		});

	auto& window = m_editor_app->GetWindow();
	m_editor_camera = std::make_shared<EditorCamera>(45.0f, 1, 0.1f, 1000.0f);
	m_editor_camera->SetPosition({ 1.5f, 0.0f, 10.0f });
	m_editor_camera->RecalculateViewMatrix();

	// Create pipeline and framebuffer (always needed)
	m_pipeline = std::make_shared<PBRPipeline>(m_renderer.GetShaderLibrary());
	
	FrameBufferSpecs specs;
	specs.Width = 1;
	specs.Height = 1;
	specs.Attachments = { TextureFormat::RGBA8, TextureFormat::Depth24Stencil8 };
	auto framebuffer = Framebuffer::Create(specs);
	m_renderer.SetFramebuffer(framebuffer);

	m_debug_renderer = std::make_unique<DebugRenderer>(m_renderer);
	m_debug_renderer->Init();
	m_overlay_renderer = std::make_unique<EditorOverlayRenderer>(*m_debug_renderer);
	
	// Register this layer with SceneManager for runtime scene transitions
	RegisterSceneLayer(this);
	
	// Get viewport panel reference for camera aspect ratio updates
	m_viewport_panel = m_editor_app->GetViewportPanel();

	// Check if project is active before loading scene
	if (!Project::GetActive())
	{
		Log::CoreInfo("No active project - EditorSceneLayer starting in empty state");
		m_editor_scene = nullptr;
		m_current_scene = nullptr;
		m_mesh = nullptr;
		return;
	}

	// Project is loaded - proceed with normal initialization
	AssetManager::LoadAssetRegistry(Project::GetActiveAssetRegistry());

	SceneSerializer scene_serializer;
	m_editor_scene = scene_serializer.Deserialize(Project::GetActiveStartScene());
	
	// Set current scene to editor scene
	m_current_scene = m_editor_scene;

	// Set the scene in the hierarchy panel
	if (auto* hierarchy_panel = m_editor_app->GetSceneHierarchyPanel())
	{
		hierarchy_panel->SetScene(m_editor_scene);
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

	m_mesh_transform_component.Scale *= 0.1;
	m_mesh_transform_component.Rotation = glm::vec3(0, glm::radians(90.0f), glm::radians(90.0f));
	
	// Connect mesh to PropertiesPanel for editing
	auto* properties_panel = m_editor_app->GetPropertiesPanel();
	if (properties_panel)
	{
		properties_panel->SetCurrentMesh(&m_mesh, &m_mesh_transform_component);
	}

	SceneSerializer().Serialize(*m_editor_scene, Project::GetActiveStartScene().replace_filename("StartSceneSaved.igscene"));
	AssetSerializer().Serialize(AssetManager::GetAssetRegistry(), Project::GetActiveAssetRegistry().replace_filename("TestARSaved.igar"));
}

void EditorSceneLayer::OnDetach()
{
	// Unregister from SceneManager
	UnregisterSceneLayer();
	
	if (m_editor_scene)
	{
		m_editor_scene->OnRuntimeStop();
	}
	m_script_module.UnregisterAll(ignis::ScriptRegistry::Get());
	m_script_module.Unload();

	AudioEngine::Get().Init();
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
		allow_camera_control = ((mouse_in_viewport && viewport_focused) ||
			m_started_camera_drag_in_viewport)
			&& !ImGuizmo::IsUsing();
	}
	
	// Edit mode: Update EditorCamera with input gating
	// (In Play mode, allow_camera_control is false, so EditorCamera doesn't update)
	if (allow_camera_control)
	{
		m_editor_camera->OnUpdate(dt);
	}

	// Update camera aspect ratio based on viewport panel size
	if (m_viewport_panel)
	{
		ImVec2 viewport_size = m_viewport_panel->GetViewportSize();
		if (viewport_size.x > 0 && viewport_size.y > 0)
		{
			float aspect = viewport_size.x / viewport_size.y;
			m_editor_camera->SetPerspective(45.0f, aspect, 0.1f, 1000.0f);
		}
	}

	auto& window = m_editor_app->GetWindow();

	// Only update runtime (scripts) in Play mode
	if (m_scene_state == SceneState::Play && m_current_scene)
	{
		m_current_scene->OnRuntimeUpdate(dt);
		
		// Process any pending scene transitions after scripts/physics update
		ProcessSceneTransition();
	}
	
	auto framebuffer = m_renderer.GetFramebuffer();

	SceneRenderer scene_renderer(m_renderer);
	if (!m_current_scene)
	{
		if (framebuffer)
		{
			framebuffer->Bind();
			m_renderer.Clear();
			framebuffer->UnBind();
		}
		return;
	}

	// In Edit mode, use EditorCamera; in Play mode, use scene camera
	std::shared_ptr<Camera> render_camera = (m_scene_state == SceneState::Edit) 
		? m_editor_camera 
		: m_current_scene->GetPrimaryCamera();

	if (!render_camera)
		render_camera = m_editor_camera;
		
	m_ui_system.OnUpdate(*m_current_scene, framebuffer->GetWidth(), framebuffer->GetHeight());

	m_renderer.BeginFrame();

	scene_renderer.BeginScene({ m_current_scene, render_camera, m_pipeline});
	m_current_scene->OnRender(scene_renderer);
	scene_renderer.EndScene();

	if (m_scene_state == SceneState::Edit)
		RenderEditorOverlay();

	m_ui_renderer.BeginUI(framebuffer->GetWidth(), framebuffer->GetHeight());
	m_ui_system.OnRender(*m_current_scene, m_ui_renderer, framebuffer->GetWidth(), framebuffer->GetHeight());
	m_ui_renderer.EndUI();

	m_renderer.EndFrame();
	
}

void EditorSceneLayer::RenderEditorOverlay()
{
	Entity selected_entity;
	if (auto* hierarchy_panel = m_editor_app->GetSceneHierarchyPanel())
		selected_entity = hierarchy_panel->GetSelectedEntity();

	m_overlay_renderer->BeginScene(m_editor_camera);

	if (selected_entity)
	{
		m_overlay_renderer->DrawColliders(selected_entity);
	}

	m_overlay_renderer->Flush();
}

	void EditorSceneLayer::OnEvent(EventBase& event)
	{
		// window resize handling removed, and viewport panel now manages framebuffer size
		// and camera aspect ratio is updated in OnUpdate() based on viewport panel size

		if (auto* e = dynamic_cast<WindowResizeEvent*>(&event))
		{
			if (m_current_scene)
			{
				auto framebuffer = m_renderer.GetFramebuffer();
				m_current_scene->OnViewportResize(framebuffer->GetWidth(), framebuffer->GetHeight());
			}
		}

		if (auto* e = dynamic_cast<KeyPressedEvent*>(&event))
		{
			if (e->GetKeyCode() == (int)KeyCode::Escape)
			{
				if (m_is_in_scene && m_scene_state == SceneState::Play)
				{
					m_is_locked = Input::IsCursorLocked();
					m_is_visible = Input::IsCursorVisible();
					Input::ShowCursor();
				}
			}
		}

		int mouse_x = Input::GetMouseX();
		int mouse_y = Input::GetMouseY();

		if (!m_viewport_panel->IsPointInViewport(mouse_x, mouse_y))
			return;

		ImVec2 min_bound = m_viewport_panel->GetViewportMinBound();

		if (auto* e = dynamic_cast<KeyTypedEvent*>(&event))
		{
			if (m_current_scene)
				m_ui_system.OnKeyTyped(*m_current_scene, e->GetKeyCode());
			return;
		}

		if (auto* e = dynamic_cast<MouseMovedEvent*>(&event))
		{
			if (m_current_scene)
				m_ui_system.OnMouseMoved(*m_current_scene,
					e->GetX() - min_bound.x, e->GetY() - min_bound.y);
		}
		else if (auto* e = dynamic_cast<MouseButtonPressedEvent*>(&event))
		{
			if (e->GetMouseButton() == 0 && m_scene_state == SceneState::Play)
			{
				if (m_is_locked)
				{
					Input::LockCursor();
				}
				else if (!m_is_visible)
				{
					Input::HideCursor();
				}
				m_is_in_scene = true;
			}

			if (e->GetMouseButton() == 1)
				m_right_pressed = true;

			if (m_current_scene)
				m_ui_system.OnMouseButtonPressed(*m_current_scene, e->GetMouseButton());
		}
		else if (auto* e = dynamic_cast<MouseButtonReleasedEvent*>(&event))
		{
			if (e->GetMouseButton() == 1)
				m_right_pressed = false;

			if (m_current_scene)
				m_ui_system.OnMouseButtonReleased(*m_current_scene, e->GetMouseButton());
		}

		if (!m_right_pressed)
		{
			if (auto* e = dynamic_cast<KeyPressedEvent*>(&event))
			{
				if (m_scene_state == SceneState::Edit)
				{
					ImGuiIO& io = ImGui::GetIO();
				
					// Check modifier
					bool modifier_pressed = (io.KeyMods & ImGuiMod_Ctrl) != 0;
				
					// Handle Cmd+C/V (macOS) or Ctrl+C/V (windows) shortcuts
					if (modifier_pressed)
					{
						auto* hierarchy_panel = m_editor_app->GetSceneHierarchyPanel();
						if (hierarchy_panel)
						{
							switch (e->GetKeyCode())
							{
							case 'C':
								hierarchy_panel->CopySelectedEntity();
								return;
							case 'V':
								hierarchy_panel->PasteEntity();
								return;
							default:
								break;
							}
						}
					}
				
					// Gizmo mode shortcuts (without modifier key)
					if (!modifier_pressed)
					{
						switch (e->GetKeyCode())
						{
						case 'Q': m_gizmo_mode = GizmoMode::None;      return;
						case 'W': m_gizmo_mode = GizmoMode::Translate; return;
						case 'E': m_gizmo_mode = GizmoMode::Rotate;    return;
						case 'R': m_gizmo_mode = GizmoMode::Scale;     return;
						default: break;
						}
					}
				}
			}
		}
	}

	void EditorSceneLayer::ReloadProject()
	{
		if (!Project::GetActive())
		{
			ClearProject();
			return;
		}
	
		Log::CoreInfo("Reloading project scene...");
	
		// Auto-stop Play mode if currently playing
		if (m_scene_state == SceneState::Play)
		{
			Log::CoreWarn("Auto-stopping Play mode for project reload");
			OnSceneStop();
		}
	
		// Clear panels Before destroying old scene to prevent accessing stale entities
		if (auto* properties_panel = m_editor_app->GetPropertiesPanel())
		{
			properties_panel->SetSelectedEntity({});
			properties_panel->SetCurrentMesh(nullptr, nullptr);
		}

		if (m_editor_scene)
		{
			m_editor_scene->OnRuntimeStop();
		}
		m_script_module.UnregisterAll(ignis::ScriptRegistry::Get());
		m_script_module.Unload();
	
		// Clear previous project's scene and assets
		m_editor_scene = nullptr;
		m_current_scene = nullptr;
		m_mesh = nullptr;

		AssetManager::ClearAll();
	
		// Reload asset registry and scene
		AssetManager::LoadAssetRegistry(Project::GetActiveAssetRegistry());
	
		// IMPORTANT: Refresh asset browser BEFORE loading scene
		if (auto* asset_browser = m_editor_app->GetAssetBrowserPanel())
		{
			asset_browser->Refresh();
			// Save asset registry after scanning to persist all imported assets
			AssetManager::SaveAssetRegistry(Project::GetActiveAssetRegistry());
		}
	
		SceneSerializer scene_serializer;
		m_editor_scene = scene_serializer.Deserialize(Project::GetActiveStartScene());

		if (!m_editor_scene)
		{
			AssetManager::ClearAll();
			Log::Error("Failed to load start scene");
			return;
		}

		// Script module will be loaded in OnScenePlay()
		// Do not call OnRuntimeStart() in edit mode, as scripts should only run in Play mode

		auto framebuffer = m_renderer.GetFramebuffer();
		m_editor_scene->OnViewportResize(framebuffer->GetWidth(), framebuffer->GetHeight());
	
		// Set current scene to editor scene
		m_current_scene = m_editor_scene;
		m_current_scene_path = Project::GetActiveStartScene();  // Track the start scene path

		// Update hierarchy panel with all entities from the scene
		if (auto* hierarchy_panel = m_editor_app->GetSceneHierarchyPanel())
		{
			hierarchy_panel->SetScene(m_editor_scene);
		}
	
		Log::CoreInfo("Project scene reloaded");
	}

	void EditorSceneLayer::LoadScene(const std::filesystem::path& scene_path)
	{
		if (!Project::GetActive())
		{
			Log::CoreError("Cannot load scene: No project is loaded");
			return;
		}

		if (!std::filesystem::exists(scene_path))
		{
			Log::CoreError("Cannot load scene: File does not exist: {}", scene_path.string());
			return;
		}

		Log::CoreInfo("Loading scene: {}", scene_path.string());

		// Auto-save current scene before switching
		if (m_editor_scene && !m_current_scene_path.empty())
		{
			SaveCurrentScene();
		}

		// Auto-stop Play mode if currently playing
		if (m_scene_state == SceneState::Play)
		{
			Log::CoreWarn("Auto-stopping Play mode for scene load");
			OnSceneStop();
		}

		// Clear panels before destroying old scene to prevent accessing stale entities
		if (auto* properties_panel = m_editor_app->GetPropertiesPanel())
		{
			properties_panel->SetSelectedEntity({});
			properties_panel->SetCurrentMesh(nullptr, nullptr);
		}

		// Clear the old editor scene (don't call OnRuntimeStop - that's for runtime scenes only)
		if (m_editor_scene)
		{
			m_editor_scene.reset();
		}

		// Clear current scene pointer
		m_current_scene = nullptr;

		// Deserialize the new scene
		SceneSerializer scene_serializer;
		auto new_scene = scene_serializer.Deserialize(scene_path);

		if (!new_scene)
		{
			Log::CoreError("Failed to load scene from: {}", scene_path.string());
			return;
		}

		// Replace the editor scene
		m_editor_scene = new_scene;
		m_current_scene = m_editor_scene;
		m_current_scene_path = scene_path;  // Track the loaded scene's file path

		// Update viewport size
		auto framebuffer = m_renderer.GetFramebuffer();
		m_editor_scene->OnViewportResize(framebuffer->GetWidth(), framebuffer->GetHeight());

		// Update hierarchy panel with entities from the new scene
		if (auto* hierarchy_panel = m_editor_app->GetSceneHierarchyPanel())
		{
			hierarchy_panel->SetScene(m_editor_scene);
		}

		Log::CoreInfo("Scene loaded successfully: {}", scene_path.string());
	}

	void EditorSceneLayer::SaveCurrentScene()
	{
		if (!m_editor_scene)
		{
			Log::CoreWarn("Cannot save scene: No scene is loaded");
			return;
		}
		
		if (m_current_scene_path.empty())
		{
			Log::CoreWarn("Cannot save scene: Scene has no file path");
			return;
		}
		
		// Always save the editor scene, not the runtime scene
		SceneSerializer serializer;
		if (serializer.Serialize(*m_editor_scene, m_current_scene_path))
		{
			Log::CoreInfo("Scene saved: {}", m_current_scene_path.string());
		}
		else
		{
			Log::CoreError("Failed to save scene: {}", m_current_scene_path.string());
		}
	}

	void EditorSceneLayer::OnScenePlay()
	{
		Log::CoreInfo("OnScenePlay() - Transitioning to Play mode");
	
		// Store original editor scene path to restore on Stop
		m_original_editor_scene_path = m_current_scene_path;
	
		// Change state to Play
		m_scene_state = SceneState::Play;
	
		// Create runtime scene and copy editor scene
		m_runtime_scene = std::make_shared<Scene>();
		m_editor_scene->CopyTo(m_runtime_scene);
	
		// Load and register script module
		m_script_module.Load(Project::ResolveActiveScriptModulePath());
		m_script_module.RegisterAll(ScriptRegistry::Get());
	
		// Start runtime (creates script instances, calls OnCreate)
		m_runtime_scene->OnRuntimeStart();
	
		// Switch to runtime scene
		m_current_scene = m_runtime_scene;

		m_is_in_scene = true;
	
		// Update hierarchy panel to use runtime scene
		if (auto* hierarchy_panel = m_editor_app->GetSceneHierarchyPanel())
		{
			hierarchy_panel->SetScene(m_runtime_scene);
		}
	
		Log::CoreInfo("OnScenePlay() - Play mode started");
	}

	void EditorSceneLayer::OnSceneStop()
	{
		Log::CoreInfo("OnSceneStop() - Transitioning to Edit mode");
	
		// Clear selected entity in PropertiesPanel before destroying runtime scene
		// This prevents accessing components on entities from destroyed registry
		if (auto* properties_panel = m_editor_app->GetPropertiesPanel())
		{
			properties_panel->SetSelectedEntity({});
		}
	
		// Stop runtime (calls OnDestroy, clears scripts)
		if (m_runtime_scene)
		{
			m_runtime_scene->OnRuntimeStop();
		}
	
		// Unload script module
		m_script_module.UnregisterAll(ScriptRegistry::Get());
		m_script_module.Unload();
	
		// Discard runtime scene
		m_runtime_scene = nullptr;
	
		// Change state to Edit
		m_scene_state = SceneState::Edit;
	
		// Load back the original editor scene (the one active when Play was clicked)
		if (!m_original_editor_scene_path.empty() && m_original_editor_scene_path != m_current_scene_path)
		{
			Log::CoreInfo("Restoring original editor scene: {}", m_original_editor_scene_path.string());
			
			// Load the original scene
			SceneSerializer scene_serializer;
			auto original_scene = scene_serializer.Deserialize(m_original_editor_scene_path);
			
			if (original_scene)
			{
				m_editor_scene = original_scene;
				m_current_scene_path = m_original_editor_scene_path;
			}
			else
			{
				Log::CoreError("Failed to restore original editor scene");
			}
		}
	
		// Switch back to editor scene
		m_current_scene = m_editor_scene;
	
		// Update hierarchy panel to use editor scene
		if (auto* hierarchy_panel = m_editor_app->GetSceneHierarchyPanel())
		{
			hierarchy_panel->SetScene(m_editor_scene);
		}
	
		Log::CoreInfo("OnSceneStop() - Edit mode restored");
	}

	Entity EditorSceneLayer::GetSelectedEntity() const
	{
		if (auto* hierarchy = m_editor_app->GetSceneHierarchyPanel())
			return hierarchy->GetSelectedEntity();
		return {};
	}

	void EditorSceneLayer::ClearProject()
	{
		if (m_editor_scene)
		{
			m_editor_scene->OnRuntimeStop();
		}
		m_script_module.UnregisterAll(ignis::ScriptRegistry::Get());
		m_script_module.Unload();

		m_editor_scene = nullptr;
		m_current_scene = nullptr;
		m_mesh = nullptr;
	
		// Clear panels
		if (auto* hierarchy_panel = m_editor_app->GetSceneHierarchyPanel())
		{
			hierarchy_panel->SetScene(nullptr);
		}
	
		if (auto* properties_panel = m_editor_app->GetPropertiesPanel())
		{
			properties_panel->SetSelectedEntity({});
			properties_panel->SetCurrentMesh(nullptr, nullptr);
		}

		AssetManager::ClearAll();
	
		Log::CoreInfo("Project scene cleared");
	}

	void EditorSceneLayer::OnScriptsReload()
	{
		m_script_module.UnregisterAll(ScriptRegistry::Get());
		m_script_module.Unload();
	}

	// ISceneLayer interface implementation for SceneManager
	void EditorSceneLayer::QueueSceneTransition(const std::filesystem::path& scene_path)
	{
		if (m_scene_state != SceneState::Play)
		{
			Log::CoreWarn("SceneManager::LoadScene can only be called during Play mode");
			return;
		}

		m_pending_scene_path = scene_path;
		Log::CoreInfo("Scene transition queued: {}", scene_path.string());
	}

	std::string EditorSceneLayer::GetCurrentSceneName() const
	{
		if (m_current_scene)
		{
			return m_current_scene->GetName();
		}
		return "";
	}

	bool EditorSceneLayer::HasPendingSceneTransition() const
	{
		return !m_pending_scene_path.empty();
	}

	void EditorSceneLayer::ProcessSceneTransition()
	{
		if (m_pending_scene_path.empty())
		{
			return;
		}

		Log::CoreInfo("Processing runtime scene transition to: {}", m_pending_scene_path.string());

		// Resolve path relative to project directory
		std::filesystem::path scene_path = m_pending_scene_path;
		if (scene_path.is_relative())
		{
			scene_path = Project::GetActiveProjectDirectory() / scene_path;
		}

		if (!std::filesystem::exists(scene_path))
		{
			Log::CoreError("Scene file does not exist: {}", scene_path.string());
			m_pending_scene_path.clear();
			return;
		}

		// Clear panels before destroying runtime scene
		if (auto* properties_panel = m_editor_app->GetPropertiesPanel())
		{
			properties_panel->SetSelectedEntity({});
		}

		// Stop current runtime scene
		if (m_runtime_scene)
		{
			m_runtime_scene->OnRuntimeStop();
		}

		// Unload script module
		m_script_module.UnregisterAll(ScriptRegistry::Get());
		m_script_module.Unload();

		// Load new scene
		SceneSerializer scene_serializer;
		auto new_scene = scene_serializer.Deserialize(scene_path);

		if (!new_scene)
		{
			Log::CoreError("Failed to load scene: {}", scene_path.string());
			m_pending_scene_path.clear();
			return;
		}

		// Replace runtime scene
		m_runtime_scene = new_scene;
		m_current_scene = m_runtime_scene;
		m_current_scene_path = scene_path;

		// Reload and register script module
		m_script_module.Load(Project::ResolveActiveScriptModulePath());
		m_script_module.RegisterAll(ScriptRegistry::Get());

		// Start new runtime scene
		m_runtime_scene->OnRuntimeStart();

		// Update viewport size
		auto framebuffer = m_renderer.GetFramebuffer();
		m_runtime_scene->OnViewportResize(framebuffer->GetWidth(), framebuffer->GetHeight());

		// Update hierarchy panel
		if (auto* hierarchy_panel = m_editor_app->GetSceneHierarchyPanel())
		{
			hierarchy_panel->SetScene(m_runtime_scene);
		}

		Log::CoreInfo("Runtime scene transition complete: {}", scene_path.filename().string());

		// Clear pending path
		m_pending_scene_path.clear();
	}

} // namespace ignis