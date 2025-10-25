#include "Sandbox.h"
#include "SandboxLayer.h"

std::unique_ptr<ignis::Application> ignis::Application::Create()
{
	return std::make_unique<Sandbox>();
}

Sandbox::Sandbox()
{
	ignis::VFS::Mount("assets", "assets");

	// VFS Test
	ignis::Log::CoreInfo("\n=== VFS Test Start===");

	// Test 1: Write a test file
	std::string test_data = "Hello from VFS!\nTest successful.";
	if (ignis::VFS::WriteText("assets://vfs_test.txt", test_data))
	{
		ignis::Log::CoreInfo("Write test passed");

		// Test 2: Read it back
		std::string read_data = ignis::VFS::ReadText("assets://vfs_test.txt");
		if (read_data == test_data)
		{
			ignis::Log::CoreInfo("Read test passed");
		}

		// Test 3: Check file exists
		if (ignis::VFS::Exists("assets://vfs_test.txt"))
		{
			ignis::Log::CoreInfo("Exists check passed");
		}

		// Test 4: List directory
		auto files = ignis::VFS::ListFiles("assets://");
		ignis::Log::CoreInfo("Found {} files in assets/", files.size());
	}

	// Test 5: Cleanup - Delete test file
	auto test_file_path = ignis::VFS::Resolve("assets://vfs_test.txt");
	if (ignis::FileSystem::Delete(test_file_path))
	{
		ignis::Log::CoreInfo("Cleanup: Test file deleted");
	}

	ignis::Log::CoreInfo("=== VFS Test End ===\n");

	ignis::Log::CoreInfo("VFS initialized");

	// Test logging system
	ignis::Log::CoreInfo("Ignis Engine initialized!");
	ignis::Log::CoreInfoTag("Core", "Application constructor called");
	ignis::Log::Info("Client application starting...");
	ignis::Log::WarnTag("Test", "This is a warning message with tag");
	ignis::Log::Error("This is an error message");

	PushLayer(std::make_unique<SandBoxLayer>(GetRenderer()));
}