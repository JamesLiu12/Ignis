#include "pch.h"
#include "Application.h"

namespace ignis {

	Application::Application()
	{
		// Initialize logging system
		Log::Init();
		
		// Test logging system
		Log::core_info("Ignis Engine initialized!");
		Log::core_info_tag("Core", "Application constructor called");
		Log::info("Client application starting...");
		Log::warn_tag("Test", "This is a warning message with tag");
		Log::error("This is an error message");
	}

	Application::~Application()
	{
		Log::core_info("Application shutting down...");
		Log::Shutdown();
	}

	void Application::Run()
	{
		Log::core_info_tag("Core", "Application main loop started");
		
		// Simple application loop with sleep to prevent 100% CPU usage
		while (m_Running)
		{
			// Sleep for a short time to prevent busy waiting
			std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
		}
		
		Log::core_info_tag("Core", "Application main loop ended");
	}
}