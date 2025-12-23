#include "Sandbox.h"
#include "SandboxLayer.h"

std::unique_ptr<ignis::Application> ignis::Application::Create()
{
	return std::make_unique<Sandbox>();
}

Sandbox::Sandbox()
{
	// Mount assets directory relative to executable location
	// This ensures assets are found regardless of current working directory
	
	// Get the executable's actual path (not current working directory)
	std::filesystem::path exe_dir;
	
	#if defined(__APPLE__)
		// macOS: Use _NSGetExecutablePath
		char path[1024];
		uint32_t size = sizeof(path);
		if (_NSGetExecutablePath(path, &size) == 0) {
			exe_dir = std::filesystem::path(path).parent_path();
		} else {
			exe_dir = std::filesystem::current_path(); // Fallback
		}
	#elif defined(_WIN32)
		// Windows: Use GetModuleFileName
		char path[MAX_PATH];
		GetModuleFileNameA(NULL, path, MAX_PATH);
		exe_dir = std::filesystem::path(path).parent_path();
	#else
		// Linux: Read /proc/self/exe
		char path[1024];
		ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
		if (len != -1) {
			path[len] = '\0';
			exe_dir = std::filesystem::path(path).parent_path();
		} else {
			exe_dir = std::filesystem::current_path(); // Fallback
		}
	#endif
	
	ignis::Log::CoreInfo("Executable directory: {}", exe_dir.string());
	
	// Try to find the assets directory
	// Executable is in: out/build/arm64-debug/Sandbox/
	// Assets are in: Sandbox/assets/
	// So we need to go up 4 levels and then into Sandbox/assets
	std::filesystem::path assets_path;
	
	// Try: exe_dir/../../../Sandbox/assets (from out/build/arm64-debug/Sandbox/ -> Sandbox/assets/)
	auto project_root = exe_dir.parent_path().parent_path().parent_path().parent_path();
	assets_path = project_root / "Sandbox" / "assets";
	
	if (std::filesystem::exists(assets_path))
	{
		ignis::Log::CoreInfo("Found assets at: {}", assets_path.string());
		ignis::VFS::Mount("assets", assets_path.string());
	}
	else
	{
		ignis::Log::CoreError("Failed to find assets directory! Tried: {}", assets_path.string());
		ignis::Log::CoreError("Executable directory: {}", exe_dir.string());
		ignis::Log::CoreError("Project root: {}", project_root.string());
		// Fallback to relative path
		ignis::VFS::Mount("assets", "Sandbox/assets");
	}

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