#include "Editor/EditorApp.h"
#include "Editor/EditorLayer.h"
#include "Editor/Panels/EditorConsolePanel.h"
#include "Editor/Panels/PropertiesPanel.h"
#include "Editor/Panels/SceneHierarchyPanel.h"
#include "Editor/Panels/EngineStatsPanel.h"
#include "Editor/Panels/PhysicsDebugPanel.h"
#include "Editor/Core/EditorConsoleSink.h"
#include "EditorSceneLayer.h"

namespace ignis {

EditorApp::EditorApp()
{
	// VFS tests (useful for development)
	Log::CoreInfo("=== VFS Test Start ===");
	Log::CoreInfo("VFS Test: Mounting 'assets' to 'assets'");
	VFS::Mount("assets", "assets");
	
	auto test_path = VFS::Resolve("assets://shaders/blinn.glsl");
	Log::CoreInfo("VFS Test: Resolved 'assets://shaders/blinn.glsl' to '{}'", test_path.string());
	Log::CoreInfo("=== VFS Test End ===\n");

	// Logging tests (useful for development)
	Log::CoreInfo("Ignis Engine initialized!");
	Log::CoreInfoTag("Core", "EditorApp constructor called");
	Log::Info("Client application starting...");
	Log::WarnTag("Test", "This is a warning message with tag");
	Log::Error("This is an error message");

	// Initialize physics system with test scene
	m_physics_world = std::make_unique<PhysicsWorld>();
	m_physics_world->Init();
	CreatePhysicsTestScene();
	Log::CoreInfo("Physics system initialized with test scene");

	// Initialize Editor Layer (manages all panels)
	auto editor_layer = std::make_unique<EditorLayer>();
	m_editor_layer = editor_layer.get();
	PushOverlay(std::move(editor_layer));

	// Register debug panels with EditorLayer's PanelManager
	auto& panel_manager = m_editor_layer->GetPanelManager();
	
	// Add Engine Stats panel (closed by default)
	auto engine_stats = panel_manager.AddPanel<EngineStatsPanel>("EngineStats", "Engine Statistics", false);
	
	// Add Physics Debug panel (closed by default)
	auto physics_debug = panel_manager.AddPanel<PhysicsDebugPanel>("PhysicsDebug", "Physics Debug", false);
	physics_debug->SetPhysicsWorld(m_physics_world.get());
	
	// Add Console panel (bottom section)
	auto console_panel = panel_manager.AddPanel<EditorConsolePanel>("Console", "Console", true);
	
	// Add EditorConsoleSink to forward logs to UI console
	auto editor_sink = std::make_shared<EditorConsoleSink>(console_panel.get());
	editor_sink->set_pattern("%v"); // Simple pattern for UI
	Log::GetCoreLogger()->sinks().push_back(editor_sink);
	Log::GetClientLogger()->sinks().push_back(editor_sink);
	
	// Add Properties panel (right section)
	m_properties_panel = panel_manager.AddPanel<PropertiesPanel>("Properties", "Properties", true);
	
	// Add Scene Hierarchy panel (left section)
	m_scene_hierarchy_panel = panel_manager.AddPanel<SceneHierarchyPanel>("SceneHierarchy", "Scene Hierarchy", true);
	m_scene_hierarchy_panel->SetPropertiesPanel(m_properties_panel.get());
	
	// Add some test messages to the console
	console_panel->AddMessage(ConsoleMessageLevel::Info, "Ignis Editor initialized");
	console_panel->AddMessage(ConsoleMessageLevel::Info, "Console panel ready");
	console_panel->AddMessage(ConsoleMessageLevel::Info, "Properties panel ready");
	
	Log::CoreInfo("Editor panels registered");
	
	// Create EditorSceneLayer with test scene content
	PushLayer(std::make_unique<EditorSceneLayer>(GetRenderer(), this));
}

EditorApp::~EditorApp()
{
	if (m_physics_world)
	{
		m_physics_world->Shutdown();
	}
}

void EditorApp::OnUpdate(float dt)
{
	// Update physics simulation
	if (m_physics_world)
	{
		m_physics_world->Step(dt);
	}
}

void EditorApp::CreatePhysicsTestScene()
{
	if (!m_physics_world) return;

	Log::CoreInfo("Creating physics test scene...");

	// Create ground plane (static body)
	RigidBodyDesc ground_desc;
	ground_desc.type = BodyType::Static;
	ground_desc.shape = ShapeType::Box;
	ground_desc.position = glm::vec3(0.0f, -1.0f, 0.0f);
	ground_desc.size = glm::vec3(10.0f, 0.2f, 10.0f);
	m_physics_world->CreateBody(ground_desc);
	Log::CoreInfo("Created ground plane");

	// Create single dynamic box
	RigidBodyDesc box_desc;
	box_desc.type = BodyType::Dynamic;
	box_desc.shape = ShapeType::Box;
	box_desc.position = glm::vec3(0.0f, 5.0f, 0.0f);
	box_desc.size = glm::vec3(1.0f, 1.0f, 1.0f);
	box_desc.mass = 1.0f;
	box_desc.friction = 0.5f;
	box_desc.restitution = 0.4f;
	m_physics_world->CreateBody(box_desc);
	Log::CoreInfo("Created dynamic box");

	Log::CoreInfo("Physics test scene created: 1 ground + 1 box");
}

} // namespace ignis

// Factory method implementation
std::unique_ptr<ignis::Application> ignis::Application::Create()
{
	return std::make_unique<ignis::EditorApp>();
}
