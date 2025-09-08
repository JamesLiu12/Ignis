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
		while (m_Running)
		{
			// Sleep for a short time to prevent busy waiting
			std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
		}
		
		Log::CoreInfoTag("Core", "Application main loop ended");
	}
}