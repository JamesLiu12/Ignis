#include "EditorApp.h"

// Factory method required by EntryPoint.cpp
std::unique_ptr<ignis::Application> ignis::Application::Create()
{
	return std::make_unique<EditorApp>();
}

EditorApp::EditorApp()
{
	// Mount VFS for assets
	ignis::VFS::Mount("assets", "assets");
	
	ignis::Log::CoreInfo("EditorApp initialized");
	ignis::Log::CoreInfo("Editor is ready - empty scene");
}
