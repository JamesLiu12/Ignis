#include "pch.h"
#include "Application.h"
#include "Ignis/ImGui/ImGuiLayer.h"
#include "Ignis/Debug/EngineStatsPanel.h"

namespace ignis 
{
	Application::Application()
	{
		s_instance = this;

		// Initialize logging system
		Log::Init();
		
		// Test logging system
		Log::CoreInfo("Ignis Engine initialized!");
		Log::CoreInfoTag("Core", "Application constructor called");
		Log::Info("Client application starting...");
		Log::WarnTag("Test", "This is a warning message with tag");
		Log::Error("This is an error message");

		m_window = Window::Create();
		m_window->SetEventCallback([this](EventBase& e) { OnEvent(e); });

		// Initialize ImGui layer
		m_imgui_layer = ImGuiLayer::Create();
		PushOverlay(std::unique_ptr<Layer>(m_imgui_layer.release()));

		// Create debug panel (starts hidden by default)
		m_debug_panel = std::make_unique<EngineStatsPanel>();

		m_subscriptions.emplace_back(
			m_dispatcher.Subscribe<WindowCloseEvent>(
				[this](WindowCloseEvent& e) { OnWindowClose(e); }
			)
		);
		m_subscriptions.emplace_back(
			m_dispatcher.Subscribe<WindowResizeEvent>(
				[this](WindowResizeEvent& e) { OnWindowResize(e); }
			)
		);
	}

	Application::~Application()
	{
		Log::CoreInfo("Application shutting down...");
		Log::Shutdown();
	}

	void Application::Run()
	{
		Log::CoreInfoTag("Core", "Application main loop started");
		
		while (m_running)
		{
			// Update all layers
			for (auto& layer : m_layer_stack)
			{
				layer->OnUpdate();
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

				// Render all layer ImGui
				for (auto& layer : m_layer_stack)
				{
					layer->OnImGuiRender();
				}

				m_imgui_layer->End();
			}

			m_window->OnUpdate();
		}
		
		Log::CoreInfoTag("Core", "Application main loop ended");
	}

	void Application::OnEvent(EventBase& e)
	{
		Log::CoreInfoTag("Core", "Event received in Application");
		
		for (auto it = m_layer_stack.rbegin(); it != m_layer_stack.rend(); ++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}

		m_dispatcher.Dispatch(e);
	}

	void Application::OnWindowClose(WindowCloseEvent& e)
	{
		Log::CoreInfoTag("Core", "Window close event received");
		m_running = false;
	}

	void Application::OnWindowResize(WindowResizeEvent& e)
	{
		Log::CoreInfoTag("Core", "Window resize event received");
	}

	void Application::PushLayer(std::unique_ptr<Layer> layer)
	{
		m_layer_stack.PushLayer(std::move(layer));
	}

	void Application::PushOverlay(std::unique_ptr<Layer> overlay)
	{
		m_layer_stack.PushOverlay(std::move(overlay));
	}
}