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
	// Mount VFS for resources FIRST (before renderer init needs shaders)
	VFS::Mount("resources", "resources");
	
	// Initialize renderer
	m_renderer.Init();
	AudioEngine::Get().Init();
	
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
	
	// Process queued scene transitions
	auto queue = m_post_scene_update_queue;
	m_post_scene_update_queue.clear();
	for (auto& fn : queue)
		fn();
}

void RuntimeSceneLayer::OnEvent(EventBase& event)
{
	if (!m_runtime_scene)
		return;
	
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
	
	// Load and register script module
	m_script_module.Load(Project::ResolveActiveScriptModulePath());
	m_script_module.RegisterAll(ScriptRegistry::Get());
	
	// Start scene
	m_runtime_scene->OnRuntimeStart();
	
	Log::CoreInfo("Runtime scene loaded: {}", scene_path.string());
}

void RuntimeSceneLayer::QueueSceneTransition(const std::filesystem::path& scene_path)
{
	m_post_scene_update_queue.push_back([this, scene_path]()
	{
		LoadScene(scene_path);
	});
}

} // namespace ignis
