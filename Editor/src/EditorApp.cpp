#include "EditorApp.h"
#include "Ignis/Editor/EditorLayer.h"
#include "Ignis/Editor/EditorConsolePanel.h"
#include "Ignis/Editor/PropertiesPanel.h"
#include "Ignis/Editor/SceneHierarchyPanel.h"
#include "Ignis/Debug/EngineStatsPanel.h"
#include "Ignis/Core/EditorConsoleSink.h"

// Factory method required by EntryPoint.cpp
std::unique_ptr<ignis::Application> ignis::Application::Create()
{
	return std::make_unique<EditorApp>();
}

EditorApp::EditorApp()
{
	// Mount VFS for assets
	ignis::VFS::Mount("assets", "assets");
	
	ignis::Log::CoreInfo("EditorApp initialized");

	// Initialize Editor Layer (manages all panels and scene)
	auto editor_layer = std::make_unique<ignis::EditorLayer>();
	m_editor_layer = editor_layer.get();
	PushOverlay(std::move(editor_layer));

	// Register debug panels with EditorLayer's PanelManager
	auto& panel_manager = m_editor_layer->GetPanelManager();
	
	// Add Engine Stats panel (closed by default)
	auto engine_stats = panel_manager.AddPanel<ignis::EngineStatsPanel>("EngineStats", "Engine Statistics", false);
	
	// Add Console panel (bottom section)
	auto console_panel = panel_manager.AddPanel<ignis::EditorConsolePanel>("Console", "Console", true);
	
	// Add EditorConsoleSink to forward logs to UI console
	auto editor_sink = std::make_shared<ignis::EditorConsoleSink>(console_panel.get());
	editor_sink->set_pattern("%v"); // Simple pattern for UI
	ignis::Log::GetCoreLogger()->sinks().push_back(editor_sink);
	ignis::Log::GetClientLogger()->sinks().push_back(editor_sink);
	
	// Add Properties panel (right section)
	m_properties_panel = panel_manager.AddPanel<ignis::PropertiesPanel>("Properties", "Properties", true);
	
	// Add Scene Hierarchy panel (left section)
	m_scene_hierarchy_panel = panel_manager.AddPanel<ignis::SceneHierarchyPanel>("SceneHierarchy", "Scene Hierarchy", true);
	m_scene_hierarchy_panel->SetPropertiesPanel(m_properties_panel.get());
	
	// Connect panels to EditorLayer for scene/mesh access
	m_editor_layer->SetPropertiesPanel(m_properties_panel.get());
	m_editor_layer->SetSceneHierarchyPanel(m_scene_hierarchy_panel.get());
	
	// Add some welcome messages to the console
	console_panel->AddMessage(ignis::ConsoleMessageLevel::Info, "Ignis Editor initialized");
	console_panel->AddMessage(ignis::ConsoleMessageLevel::Info, "Console panel ready");
	console_panel->AddMessage(ignis::ConsoleMessageLevel::Info, "Properties panel ready");
	console_panel->AddMessage(ignis::ConsoleMessageLevel::Info, "Scene ready - use Properties panel to load a model");
	
	ignis::Log::CoreInfo("Editor panels registered");
	ignis::Log::CoreInfo("Editor is ready");
}

EditorApp::~EditorApp()
{
	// Remove EditorConsoleSink from loggers before cleanup
	// This prevents dangling pointer issues when panels are destroyed
	auto& core_sinks = ignis::Log::GetCoreLogger()->sinks();
	auto& client_sinks = ignis::Log::GetClientLogger()->sinks();
	
	// Remove the last sink (EditorConsoleSink) that we added in constructor
	if (!core_sinks.empty()) {
		core_sinks.pop_back();
	}
	if (!client_sinks.empty()) {
		client_sinks.pop_back();
	}
	
	// Clean up panels before base Application destructor runs
	m_scene_hierarchy_panel.reset();
	m_properties_panel.reset();
	m_editor_layer = nullptr;
}
