#include "EditorLayer.h"
#include "Ignis/Core/Log.h"
#include "Ignis/Core/Application.h"
#include "Ignis/Asset/AssetManager.h"
#include "Ignis/Renderer/Renderer.h"
#include "Ignis/Renderer/PBRPipeline.h"
#include <imgui.h>

namespace ignis {

	EditorLayer::EditorLayer()
		: Layer("EditorLayer")
	{
	}

	void EditorLayer::OnAttach()
	{
		Log::CoreInfo("EditorLayer attached");

		// Create panel manager
		m_panel_manager = std::make_unique<PanelManager>();
		
		// Initialize scene and rendering
		InitializeScene();
		
		// Connect scene and mesh to panels (will be called after panels are created)
		// This is done in a separate step after EditorApp creates the panels
		
		Log::CoreInfo("EditorLayer initialized with PanelManager and Scene");
	}
	
	void EditorLayer::SetPropertiesPanel(PropertiesPanel* panel)
	{
		m_properties_panel = panel;
		
		// Connect mesh to properties panel for editing
		if (m_properties_panel)
		{
			m_properties_panel->SetCurrentMesh(&m_mesh, &m_mesh_transform);
			Log::CoreInfo("Connected mesh to PropertiesPanel");
		}
	}
	
	void EditorLayer::SetSceneHierarchyPanel(SceneHierarchyPanel* panel)
	{
		m_scene_hierarchy_panel = panel;
		
		// Connect scene to hierarchy panel
		if (m_scene_hierarchy_panel && m_scene)
		{
			m_scene_hierarchy_panel->SetScene(m_scene);
			Log::CoreInfo("Connected scene to SceneHierarchyPanel");
		}
	}

	void EditorLayer::OnDetach()
	{
		// Don't log during detach - logging may already be shut down during application cleanup
	}

	void EditorLayer::OnUpdate(float dt)
	{
		// Update camera
		if (m_camera)
		{
			m_camera->OnUpdate(dt);
		}
		
		// Render scene
		if (m_pipeline && m_scene && m_camera)
		{
			auto& renderer = Application::Get().GetRenderer();
			renderer.Clear();
			renderer.BeginScene(m_pipeline, m_scene, m_camera);
			
			// Render mesh if loaded
			if (m_mesh)
			{
				renderer.RenderMesh(m_mesh, m_mesh_transform.GetTransform());
			}
			
			renderer.EndScene();
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		// Render menu bar
		RenderMenuBar();

		// Render all panels managed by PanelManager
		m_panel_manager->OnImGuiRender();
	}

	void EditorLayer::OnEvent(EventBase& event)
	{
		// Handle camera events first
		if (m_camera)
		{
			// Camera handles mouse/keyboard input for navigation
			// (EditorCamera internally handles events via Input system)
		}
		
		// Handle window resize
		if (auto* resize_event = dynamic_cast<WindowResizeEvent*>(&event))
		{
			if (m_camera)
			{
				float aspect_ratio = static_cast<float>(resize_event->GetWidth()) / 
				                     static_cast<float>(resize_event->GetHeight());
				m_camera->SetPerspective(45.0f, aspect_ratio, 0.1f, 1000.0f);
				m_camera->RecalculateViewMatrix();
			}
		}
		
		// Forward events to all panels
		m_panel_manager->OnEvent(event);
	}
	
	void EditorLayer::InitializeScene()
	{
		// Create shader library and load PBR shader
		m_shader_library = std::make_shared<ShaderLibrary>();
		m_shader_library->Load("assets://shaders/blinn.glsl");
		
		// Create editor camera
		m_camera = std::make_shared<EditorCamera>(45.0f, 1280.0f / 720.0f, 0.1f, 1000.0f);
		m_camera->SetPosition({ 0.0f, 0.0f, 5.0f });
		m_camera->RecalculateViewMatrix();
		
		// Create empty scene (no entities)
		m_scene = std::make_shared<Scene>();
		
		// Create PBR pipeline
		m_pipeline = std::make_shared<PBRPipeline>(m_shader_library);
		
		// Initialize mesh transform (no mesh loaded yet)
		m_mesh_transform.Translation = glm::vec3(0.0f, 0.0f, 0.0f);
		m_mesh_transform.Rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		m_mesh_transform.Scale = glm::vec3(1.0f, 1.0f, 1.0f);
		
		Log::CoreInfo("Editor scene initialized - empty scene ready for development");
	}

	void EditorLayer::RenderMenuBar()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("View"))
			{
				// Toggle panels on/off
				auto& panels = m_panel_manager->GetPanels();
				for (auto& panel_data : panels)
				{
					ImGui::MenuItem(panel_data.Name.c_str(), nullptr, &panel_data.IsOpen);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem("About Ignis Editor"))
				{
					Log::CoreInfo("Ignis Editor - Game Engine Editor");
				}
				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}
	}

} // namespace ignis