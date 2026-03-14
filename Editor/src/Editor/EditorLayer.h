#pragma once

#include "Ignis.h"
#include "Editor/Panels/PanelManager.h"

namespace ignis {

	// Main editor layer that manages all editor panels and UI
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(float dt) override;
		void OnImGuiRender() override;
		void OnEvent(EventBase& event) override;

		PanelManager& GetPanelManager() { return *m_panel_manager; }

		// Project management methods
		void OpenProject();  // Shows folder dialog
		void OpenProject(const std::filesystem::path& filepath);  // Loads project from path
		void SaveProject();  // Saves to current project path
		void SaveProject(const std::filesystem::path& filepath);  // Saves to specific path
		void SaveProjectAs();  // Shows folder dialog, saves to new location
		void CloseProject();  // Closes current project
		void CreateNewProject();  // Shows new project popup

		void BuildScripts();

		// Scene management methods
		void NewScene();  // Shows new scene popup
		void LoadScene();  // Shows load scene dialog
		void SaveScene();  // Saves current scene
		void SaveSceneAs();  // Shows save scene as dialog
		void SetCurrentSceneAsStartScene();  // Sets current scene as project start scene

		// Export functionality
		void ExportGame();

	private:
		void RenderMenuBar();
		void UI_ShowNewProjectPopup();
		void UI_ShowNewScenePopup();
		void HandleKeyboardShortcuts();
	
		// Deferred operation helpers
		void ProcessDeferredProjectLoad();
		void ProcessDeferredSaveAs();
		void ProcessDeferredSceneLoad();
		void ProcessDeferredSceneSaveAs();

	private:
		std::unique_ptr<PanelManager> m_panel_manager;

		// Deferred loading buffers (safe to load outside ImGui rendering)
		static char s_OpenProjectFilePathBuffer[512];
		static char s_SaveProjectAsFolderBuffer[512];
		static char s_NewProjectFolderBuffer[512];
		static char s_NewProjectNameBuffer[128];
		static bool s_ShowNewProjectPopup;

		// Scene management buffers
		static char s_LoadSceneFilePathBuffer[512];
		static char s_SaveSceneAsFolderBuffer[512];
		static char s_NewSceneFolderBuffer[512];
		static char s_NewSceneNameBuffer[128];
		static bool s_ShowNewScenePopup;
	};

} // namespace ignis