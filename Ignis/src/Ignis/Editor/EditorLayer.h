#pragma once

#include "Ignis/Core/Layer.h"
#include "PanelManager.h"
#include <memory>

namespace ignis {

	/// <summary>
	/// Main editor layer that manages all editor panels and UI
	/// This layer is pushed as an overlay in the Application
	/// </summary>
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

		/// <summary>
		/// Get the panel manager
		/// </summary>
		PanelManager& GetPanelManager() { return *m_panel_manager; }

	private:
		void RenderMenuBar();

	private:
		std::unique_ptr<PanelManager> m_panel_manager;
	};

} // namespace ignis