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

		void OpenProject(const std::filesystem::path& filepath);
		void SaveProject(const std::filesystem::path& filepath);

	private:
		void RenderMenuBar();

	private:
		std::unique_ptr<PanelManager> m_panel_manager;
	};

} // namespace ignis