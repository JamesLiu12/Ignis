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
}