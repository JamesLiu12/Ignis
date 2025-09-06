#include "pch.h"
#include "Application.h"
#include <thread>
#include <chrono>

namespace ignis {

	Application::Application()
	{
		// Initialize logging system
		Log::Init();
		
		// Test logging system
		IG_CORE_INFO("Ignis Engine initialized!");
		IG_CORE_INFO_TAG("Core", "Application constructor called");
		IG_INFO("Client application starting...");
		IG_WARN_TAG("Test", "This is a warning message with tag");
		IG_ERROR("This is an error message");
	}

	Application::~Application()
	{
		IG_CORE_INFO("Application shutting down...");
		Log::Shutdown();
	}

	void Application::Run()
	{
		IG_CORE_INFO_TAG("Core", "Application main loop started");
		
		// Simple application loop with sleep to prevent 100% CPU usage
		while (m_Running)
		{
			// Sleep for a short time to prevent busy waiting
			std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
		}
		
		IG_CORE_INFO_TAG("Core", "Application main loop ended");
	}
}