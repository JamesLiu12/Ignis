#pragma once

#include "Editor/Panels/EditorPanel.h"
#include "Ignis/Renderer/Renderer.h"
#include <imgui.h>

namespace ignis {

	class ViewportPanel : public EditorPanel
	{
	public:
		ViewportPanel(Renderer* renderer);
		~ViewportPanel() override = default;

		void OnImGuiRender() override;
		void OnEvent(EventBase& event) override;

		std::string_view GetName() const override { return "Viewport"; }
		std::string_view GetID() const override { return "viewport_panel"; }

		ImVec2 GetViewportSize() const { return m_viewport_size; }

	private:
		Renderer* m_renderer;
		ImVec2 m_viewport_size;
	};

} // namespace ignis
