#pragma once

#include "Ignis.h"
#include "Editor/PanelManager.h"

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

	private:
		void RenderMenuBar();
		void UI_ShowNewProjectPopup();
	
		// Deferred operation helpers
		void ProcessDeferredProjectLoad();
		void ProcessDeferredSaveAs();

	private:
		std::unique_ptr<PanelManager> m_panel_manager;

		// Deferred loading buffers (safe to load outside ImGui rendering)
		static char s_OpenProjectFilePathBuffer[512];
		static char s_SaveProjectAsFolderBuffer[512];
		static char s_NewProjectFolderBuffer[512];
		static char s_NewProjectNameBuffer[128];
		static bool s_ShowNewProjectPopup;
	};

} // namespace ignis