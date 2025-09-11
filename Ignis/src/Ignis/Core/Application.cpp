#include "pch.h"
#include "Application.h"

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
		
		// Simple application loop with sleep to prevent 100% CPU usage
		while (m_running)
		{
			m_window->OnUpdate();
		}
		
		Log::CoreInfoTag("Core", "Application main loop ended");
	}

	void Application::OnEvent(EventBase& e)
	{
		Log::CoreInfoTag("Core", "Event received in Application");
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
}