#include "RuntimeApp.h"
#include "RuntimeSceneLayer.h"

// Platform-specific headers for executable path detection
#if defined(__APPLE__)
	#include <mach-o/dyld.h>
#elif defined(_WIN32)
	#include <windows.h>
#elif defined(__linux__)
	#include <unistd.h>
#endif

namespace ignis {

RuntimeApp::RuntimeApp(const std::string& project_path)
	: m_project_path(project_path)
{
	Log::CoreInfo("Ignis Runtime initialized");
	Log::CoreInfo("Project path: {}", m_project_path);
	
	// Create runtime scene layer
	auto scene_layer = std::make_unique<RuntimeSceneLayer>(GetRenderer(), m_project_path);
	m_scene_layer = scene_layer.get();
	PushLayer(std::move(scene_layer));
	
	Log::CoreInfo("Runtime application ready");
}

RuntimeApp::~RuntimeApp()
{
	Log::CoreInfo("Runtime application shutting down");
}

void RuntimeApp::OnUpdate(float dt)
{
	// Scene updates happen in RuntimeSceneLayer::OnUpdate()
}

} // namespace ignis

// Helper function to get executable directory
static std::filesystem::path GetExecutableDirectory()
{
#if defined(__APPLE__)
	// macOS: Use _NSGetExecutablePath
	char path[1024];
	uint32_t size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0)
	{
		return std::filesystem::path(path).parent_path();
	}
#elif defined(_WIN32)
	// Windows: Use GetModuleFileName
	char path[MAX_PATH];
	GetModuleFileNameA(NULL, path, MAX_PATH);
	return std::filesystem::path(path).parent_path();
#elif defined(__linux__)
	// Linux: Read /proc/self/exe
	char path[1024];
	ssize_t count = readlink("/proc/self/exe", path, sizeof(path));
	if (count != -1)
	{
		path[count] = '\0';
		return std::filesystem::path(path).parent_path();
	}
#endif
	// Fallback to current directory
	return std::filesystem::current_path();
}

// Factory method implementation for Runtime
std::unique_ptr<ignis::Application> ignis::Application::Create()
{
	// Auto-detect project path from executable location
	auto exe_dir = GetExecutableDirectory();
	
	// For development: executable is in out/build/arm64-debug/bin/
	// Project root is 4 levels up
	auto project_root = exe_dir.parent_path().parent_path().parent_path().parent_path();
	
	// Check if .igproj exists in project root
	std::string project_path;
	bool found_project = false;
	for (const auto& entry : std::filesystem::directory_iterator(project_root))
	{
		if (entry.path().extension() == ".igproj")
		{
			found_project = true;
			break;
		}
	}
	
	if (found_project)
	{
		project_path = project_root.string();
	}
	else
	{
		// Fallback: assume project is in same directory as executable (distribution)
		project_path = exe_dir.string();
	}
	
	return std::make_unique<ignis::RuntimeApp>(project_path);
}
