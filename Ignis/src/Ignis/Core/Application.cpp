#include "Application.h"
#include "Ignis/ImGui/ImGuiLayer.h"
#include "Ignis/Debug/EngineStatsPanel.h"
#include "Ignis/Core/Events/KeyEvents.h"
#include "Input.h"
#include "Ignis/Physics/PhysicsWorld.h"
#include "Ignis/Debug/PhysicsDebugPanel.h"
#include "Ignis/Renderer/Camera.h"
#include "Ignis/Renderer/RendererContext.h"
#include "Ignis/Asset/VFS.h"
#include "Ignis/Core/FileSystem.h"

namespace ignis 
{
	Application::Application()
	{
		s_instance = this;

		// Initialize logging system
		Log::Init();

		// Initialize VFS
		VFS::Init();

		m_window = Window::Create();
		m_window->SetEventCallback([this](EventBase& e) { OnEvent(e); });

		glfwMakeContextCurrent(static_cast<GLFWwindow*>(m_window->GetNativeWindow()));
		std::unique_ptr<RendererContext> context = RendererContext::Create();
		context->Init();

		m_renderer = Renderer::Create();

		// Initialize ImGui layer
		auto imgui_layer = ImGuiLayer::Create();
		m_imgui_layer = imgui_layer.get();
		PushOverlay(std::move(imgui_layer));

		// Create debug panel
		m_debug_panel = std::make_unique<EngineStatsPanel>();
		Log::CoreInfo("Debug panel created successfully");
		Log::CoreInfo("Debug window initial state: {}", m_show_debug_window ? "VISIBLE" : "HIDDEN");

		// Initialize physics
		m_physics_world = std::make_unique<PhysicsWorld>();
		m_physics_world->Init();
		m_physics_debug_panel = std::make_unique<PhysicsDebugPanel>();
		CreatePhysicsTestScene();
		Log::CoreInfo("Physics system initialized");

		m_subscriptions.emplace_back(
			m_dispatcher.Subscribe<WindowResizeEvent>(
				[this](WindowResizeEvent& e) { OnWindowResize(e); }
			)
		);
	}

	Application::~Application()
	{
		if (m_physics_world)
		{
			m_physics_world->Shutdown();
		}
		Log::CoreInfo("Application shutting down...");
		VFS::Shutdown();
		Log::Shutdown();
	}

	void Application::Run()
	{
		Log::CoreInfoTag("Core", "Application main loop started");

		float last_frame_time = 0.0f;

		while (m_running)
		{
			float time = static_cast<float>(glfwGetTime());
			float delta_time = time - last_frame_time;
			last_frame_time = time;

			if (m_physics_world)
			{
				m_physics_world->Step(delta_time);
			}

			// Update all layers
			for (auto& layer : m_layer_stack)
			{
				layer->OnUpdate(delta_time);
			}

			if (Input::IsKeyPressed(KeyCode::A))
			{
				Log::CoreInfo("Key 'A' is currently pressed.");
			}

			if (Input::IsMouseButtonPressed(MouseButton::Left))
			{
				auto [x, y] = Input::GetMousePosition();
				Log::CoreInfo("Left mouse button is currently pressed at position ({}, {}).", x, y);
			}

			// ImGui rendering
			if (m_imgui_layer)
			{
				m_imgui_layer->Begin();
				
				// Render debug window only if enabled
				if (m_debug_panel && m_show_debug_window)
				{
					m_debug_panel->OnImGuiRender(m_show_debug_window);
				}

				if (m_physics_debug_panel && m_show_physics_debug)
				{
					m_physics_debug_panel->OnImGuiRender(m_physics_world.get(), m_show_physics_debug);
				}
			
				// Render all layer ImGui
				for (auto& layer : m_layer_stack)
				{
					layer->OnImGuiRender();
				}
			
				m_imgui_layer->End();
			}
			else
			{
				Log::CoreError("ImGui layer is null!");
			}

			m_window->OnUpdate();
		}
		
		Log::CoreInfoTag("Core", "Application main loop ended");
	}

	void Application::OnEvent(EventBase& e)
	{
		// Handle window close event
		if (auto* close_event = dynamic_cast<WindowCloseEvent*>(&e))
		{
			Log::CoreInfo("Window close event received in OnEvent - setting m_running to false");
			m_running = false;
			return;
		}

		// Handle F# key for debug window toggle
		if (auto* key_event = dynamic_cast<KeyPressedEvent*>(&e))
		{
			if (key_event->GetKeyCode() == 290)  // GLFW_KEY_F1 = 290
			{
				m_show_debug_window = !m_show_debug_window;
				Log::CoreInfo("Debug window toggled: {}", m_show_debug_window ? "ON" : "OFF");
			}
			else if (key_event->GetKeyCode() == 291)  // GLFW_KEY_F2 = 291
            {
                m_show_physics_debug = !m_show_physics_debug;
                Log::CoreInfo("Physics debug toggled: {}", m_show_physics_debug ? "ON" : "OFF");
            }
		
		}

		m_dispatcher.Dispatch(e);

		for (auto it = m_layer_stack.rbegin(); it != m_layer_stack.rend(); ++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}
	}

	void Application::OnWindowClose(WindowCloseEvent& e)
	{
		Log::CoreInfoTag("Core", "Window close event received");
		m_running = false;
	}

	void Application::OnWindowResize(WindowResizeEvent& e)
	{
		Log::CoreInfoTag("Core", "Window resize event received");
		m_renderer->SetViewport(0, 0, e.GetWidth(), e.GetHeight());
	}

	void Application::PushLayer(std::unique_ptr<Layer> layer)
	{
		m_layer_stack.PushLayer(std::move(layer));
	}

	void Application::PushOverlay(std::unique_ptr<Layer> overlay)
	{
		m_layer_stack.PushOverlay(std::move(overlay));
	}

	void Application::CreatePhysicsTestScene()
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
}