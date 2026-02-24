#pragma once

#include "Editor/Panels/EditorPanel.h"
#include "Ignis/Renderer/Renderer.h"
#include "Ignis/ImGui/ImGuiTextureHelper.h"
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
		ImVec2 GetViewportMinBound() const { return m_viewport_min_bound; }
		ImVec2 GetViewportMaxBound() const { return m_viewport_max_bound; }
		bool IsFocused() const { return m_is_focused; }

	private:
		Renderer* m_renderer;
		ImVec2 m_viewport_size;
		ImVec2 m_viewport_min_bound;
		ImVec2 m_viewport_max_bound;
		bool m_is_focused = false;
		
		std::unique_ptr<ImGuiTextureHelper> m_imgui_texture_helper;
	};

} // namespace ignis
