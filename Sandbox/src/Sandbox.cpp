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

	// Test 1: Open file for writing
	std::string test_data = "Hello from VFS!\nTest successful.";
	auto write_file = ignis::VFS::Open("assets://vfs_test.txt");
	if (write_file.IsOpen())
	{
		ignis::Log::CoreInfo("File opened for writing");

		// Write test data
		if (write_file.WriteText(test_data))
		{
			ignis::Log::CoreInfo("Write test passed");
		}
		else
		{
			ignis::Log::CoreError("Write test failed");
		}
	}
	else
	{
		ignis::Log::CoreError("Cannot open file for writing: {}", write_file.GetError());
	}

	// Test 2: Open file for reading
	auto read_file = ignis::VFS::Open("assets://vfs_test.txt");
	if (read_file.IsOpen())
	{
		ignis::Log::CoreInfo("File opened for reading - IsReadable: {}", read_file.IsReadable());

		std::string read_data = read_file.ReadText();
		if (read_data == test_data)
		{
			ignis::Log::CoreInfo("Read test passed");
		}
		else
		{
			ignis::Log::CoreError("Read test failed - data mismatch");
		}
	}
	else
	{
		ignis::Log::CoreError("Cannot open file for reading: {}", read_file.GetError());
	}

	// Test 3: Check file exists
	if (ignis::VFS::Exists("assets://vfs_test.txt"))
	{
		ignis::Log::CoreInfo("Exists check passed");
	}

	// Test 4: List directory
	auto files = ignis::VFS::ListFiles("assets://");
	ignis::Log::CoreInfo("Found {} files in assets/", files.size());

	// Test 5: Cleanup - Delete test file
	auto test_file_path = ignis::VFS::Resolve("assets://vfs_test.txt");
	if (ignis::FileSystem::Delete(test_file_path))
	{
		ignis::Log::CoreInfo("Cleanup: Test file deleted");
	}

	ignis::Log::CoreInfo("=== VFS Test End ===\n");

	// Test logging system
	ignis::Log::CoreInfo("Ignis Engine initialized!");
	ignis::Log::CoreInfoTag("Core", "Application constructor called");
	ignis::Log::Info("Client application starting...");
	ignis::Log::WarnTag("Test", "This is a warning message with tag");
	ignis::Log::Error("This is an error message");

	PushLayer(std::make_unique<SandBoxLayer>(GetRenderer()));
}