#include "RuntimeSceneLayer.h"
#include "Ignis/Project/Project.h"
#include "Ignis/Project/ProjectSerializer.h"
#include "Ignis/Scene/SceneSerializer.h"
#include "Ignis/Asset/AssetManager.h"
#include "Ignis/Script/ScriptRegistry.h"
#include "Ignis/Audio/AudioEngine.h"
#include "Ignis/Renderer/IBLBaker.h"
#include "Ignis/Core/Events/MouseEvents.h"
#include "Ignis/Core/Events/KeyEvents.h"
#include "Ignis/Core/Events/WindowEvents.h"

namespace ignis {

RuntimeSceneLayer::RuntimeSceneLayer(Renderer& renderer, const std::string& project_path)
	: m_renderer(renderer)
	, m_project_path(project_path)
	, m_ui_renderer(renderer)
{
}

RuntimeSceneLayer::~RuntimeSceneLayer()
{
}

void RuntimeSceneLayer::OnAttach()
{
	// Register with SceneManager for runtime scene transitions
	SceneManager::RegisterSceneLayer(this);
	
	// Mount VFS for resources FIRST (before renderer init needs shaders)
	VFS::Mount("resources", "resources");
	
	// Initialize renderer
	m_renderer.Init();
	AudioEngine::Get().Init();
	
	// Set initial viewport size using framebuffer dimensions
	auto& window = Application::Get().GetWindow();
	uint32_t fb_width = window.GetFramebufferWidth();
	uint32_t fb_height = window.GetFramebufferHeight();
	m_renderer.SetViewport(0, 0, fb_width, fb_height);
	Log::CoreInfoTag("Runtime", "Initial viewport set: {0}x{1}", fb_width, fb_height);
	
	// Set asset load context
	AssetManager::SetLoadContext({
		.IBLBakerService = IBLBaker::Create(m_renderer),
	});
	
	// Find and load project file
	std::filesystem::path project_file;
	for (const auto& entry : std::filesystem::directory_iterator(m_project_path))
	{
		if (entry.path().extension() == ".igproj")
		{
			project_file = entry.path();
			break;
		}
	}
	
	if (project_file.empty())
	{
		Log::CoreError("No .igproj file found in: {}", m_project_path);
		return;
	}
	
	// Deserialize and set active project
	ProjectSerializer serializer;
	auto project = serializer.Deserialize(project_file);
	if (!project)
	{
		Log::CoreError("Failed to load project: {}", project_file.string());
		return;
	}
	
	Project::SetActive(project);
	Log::CoreInfo("Project loaded: {}", Project::GetActiveProjectName());
	
	// Load asset registry
	AssetManager::LoadAssetRegistry(Project::GetActiveAssetRegistry());
	
	// Create pipeline
	m_pipeline = std::make_shared<PBRPipeline>(m_renderer.GetShaderLibrary());
	
	// Check if start scene is configured
	auto start_scene = Project::GetActiveStartScene();
	if (start_scene.empty())
	{
		Log::CoreError("=== Configuration Error ===");
		Log::CoreError("No start scene configured in project");
		Log::CoreError("Please set a start scene in the Editor before exporting");
		Log::CoreError("==========================");
		return;
	}
	
	// Load start scene
	LoadScene(start_scene);
	
	Log::CoreInfo("Runtime scene layer attached");
}

void RuntimeSceneLayer::OnDetach()
{
	// Unregister from SceneManager
	SceneManager::UnregisterSceneLayer();
	
	if (m_runtime_scene)
	{
		m_runtime_scene->OnRuntimeStop();
	}
	
	m_script_module.UnregisterAll(ScriptRegistry::Get());
	m_script_module.Unload();
	
	AudioEngine::Get().Shutdown();
}

void RuntimeSceneLayer::OnUpdate(float dt)
{
	if (!m_runtime_scene)
		return;
	
	// Update scene runtime (scripts)
	m_runtime_scene->OnRuntimeUpdate(dt);
	
	// Get window dimensions for UI
	auto& window = Application::Get().GetWindow();
	uint32_t window_width = window.GetWidth();
	uint32_t window_height = window.GetHeight();
	
	// Update UI layout
	m_ui_system.OnUpdate(*m_runtime_scene, window_width, window_height);
	
	// Render scene
	m_renderer.BeginFrame();
	
	auto camera = m_runtime_scene->GetPrimaryCamera();
	if (camera)
	{
		SceneRenderer scene_renderer(m_renderer);
		scene_renderer.BeginScene({ m_runtime_scene, camera, m_pipeline });
		m_runtime_scene->OnRender(scene_renderer);
		scene_renderer.EndScene();
	}
	
	// Render UI
	m_ui_renderer.BeginUI(window_width, window_height);
	m_ui_system.OnRender(*m_runtime_scene, m_ui_renderer, window_width, window_height);
	m_ui_renderer.EndUI();
	
	m_renderer.EndFrame();
	
	// Process pending scene transitions
	ProcessSceneTransition();
}

void RuntimeSceneLayer::OnEvent(EventBase& event)
{
	if (!m_runtime_scene)
		return;
	
	// Handle window resize to update viewport
	if (auto* e = dynamic_cast<WindowResizeEvent*>(&event))
	{
		auto& window = Application::Get().GetWindow();
		uint32_t fb_width = window.GetFramebufferWidth();
		uint32_t fb_height = window.GetFramebufferHeight();
		
		// Update renderer viewport to match framebuffer size
		m_renderer.SetViewport(0, 0, fb_width, fb_height);
		
		// Update camera aspect ratio
		auto camera = m_runtime_scene->GetPrimaryCamera();
		if (camera && fb_width > 0 && fb_height > 0)
		{
			float aspect = static_cast<float>(fb_width) / static_cast<float>(fb_height);
			camera->SetPerspective(45.0f, aspect, 0.1f, 1000.0f);
		}
		
		Log::CoreInfoTag("Runtime", "Viewport updated: {0}x{1}, aspect: {2}", 
			fb_width, fb_height, static_cast<float>(fb_width) / static_cast<float>(fb_height));
	}
	
	// Handle UI events using dynamic_cast to check event types
	if (auto* e = dynamic_cast<MouseMovedEvent*>(&event))
	{
		m_ui_system.OnMouseMoved(*m_runtime_scene, e->GetX(), e->GetY());
	}
	else if (auto* e = dynamic_cast<MouseButtonPressedEvent*>(&event))
	{
		m_ui_system.OnMouseButtonPressed(*m_runtime_scene, e->GetMouseButton());
	}
	else if (auto* e = dynamic_cast<MouseButtonReleasedEvent*>(&event))
	{
		m_ui_system.OnMouseButtonReleased(*m_runtime_scene, e->GetMouseButton());
	}
	else if (auto* e = dynamic_cast<KeyTypedEvent*>(&event))
	{
		m_ui_system.OnKeyTyped(*m_runtime_scene, e->GetKeyCode());
	}
}

void RuntimeSceneLayer::LoadScene(const std::filesystem::path& scene_path)
{
	// Stop current scene if exists
	if (m_runtime_scene)
	{
		m_runtime_scene->OnRuntimeStop();
		m_script_module.UnregisterAll(ScriptRegistry::Get());
		m_script_module.Unload();
	}
	
	// Load new scene
	SceneSerializer serializer;
	m_runtime_scene = serializer.Deserialize(scene_path);
	
	if (!m_runtime_scene)
	{
		Log::CoreError("Failed to load scene: {}", scene_path.string());
		return;
	}
	
	// Track current scene path
	m_current_scene_path = scene_path;
	
	// Load and register script module
	m_script_module.Load(Project::ResolveActiveScriptModulePath());
	m_script_module.RegisterAll(ScriptRegistry::Get());
	
	// Start scene
	m_runtime_scene->OnRuntimeStart();
	
	// Update camera aspect ratio to match viewport
	auto& window = Application::Get().GetWindow();
	uint32_t fb_width = window.GetFramebufferWidth();
	uint32_t fb_height = window.GetFramebufferHeight();
	m_runtime_scene->OnViewportResize(fb_width, fb_height);
	
	Log::CoreInfo("Runtime scene loaded: {}", scene_path.string());
}

// ISceneLayer interface implementation
void RuntimeSceneLayer::QueueSceneTransition(const std::filesystem::path& scene_path)
{
	m_pending_scene_path = scene_path;
	Log::CoreInfo("Scene transition queued: {}", scene_path.string());
}

std::string RuntimeSceneLayer::GetCurrentSceneName() const
{
	if (m_runtime_scene)
	{
		return m_runtime_scene->GetName();
	}
	return "";
}

bool RuntimeSceneLayer::HasPendingSceneTransition() const
{
	return !m_pending_scene_path.empty();
}

void RuntimeSceneLayer::ProcessSceneTransition()
{
	// If async loading is in progress, check if ready
	if (m_is_async_loading)
	{
		if (m_async_loader.IsReady())
		{
			Log::CoreInfo("Async scene load complete, finalizing transition");

			// Get the loaded scene
			auto new_scene = m_async_loader.GetScene();

			if (!new_scene)
			{
				Log::CoreError("Failed to load scene asynchronously");
				m_is_async_loading = false;
				m_pending_scene_path.clear();
				return;
			}

			// Resolve path relative to project directory
			std::filesystem::path scene_path = m_pending_scene_path;
			if (scene_path.is_relative())
			{
				scene_path = Project::GetActiveProjectDirectory() / scene_path;
			}

			// Replace runtime scene
			m_runtime_scene = new_scene;
			m_current_scene_path = scene_path;

			// Reload and register script module
			m_script_module.Load(Project::ResolveActiveScriptModulePath());
			m_script_module.RegisterAll(ScriptRegistry::Get());

			// Start new runtime scene
			m_runtime_scene->OnRuntimeStart();
			
			// Update camera aspect ratio to match viewport
			auto& window = Application::Get().GetWindow();
			uint32_t fb_width = window.GetFramebufferWidth();
			uint32_t fb_height = window.GetFramebufferHeight();
			m_runtime_scene->OnViewportResize(fb_width, fb_height);

			Log::CoreInfo("Runtime scene transition complete: {}", scene_path.filename().string());

			// Clear state
			m_is_async_loading = false;
			m_pending_scene_path.clear();
		}
		else
		{
			// Still loading, display progress
			float progress = m_async_loader.GetProgress();
			static int frame_count = 0;
			if (++frame_count % 60 == 0)
			{
				Log::CoreInfo("Loading scene '{}': {:.0f}%", m_loading_scene_name, progress * 100.0f);
			}
		}
		return;
	}

	// Start new async load if there's a pending scene
	if (!m_pending_scene_path.empty())
	{
		Log::CoreInfo("Starting async scene transition to: {}", m_pending_scene_path.string());

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

		// Stop current runtime scene
		if (m_runtime_scene)
		{
			m_runtime_scene->OnRuntimeStop();
		}

		// Unload script module
		m_script_module.UnregisterAll(ScriptRegistry::Get());
		m_script_module.Unload();

		// Start async load
		m_loading_scene_name = scene_path.filename().string();
		m_async_loader.LoadSceneAsync(scene_path);
		m_is_async_loading = true;

		Log::CoreInfo("Async load started for: {}", m_loading_scene_name);
	}
}

} // namespace ignis
