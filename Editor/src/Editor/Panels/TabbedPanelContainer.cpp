#include "Editor/Panels/TabbedPanelContainer.h"
#include <imgui.h>

namespace ignis {

	void TabbedPanelContainer::AddTab(const std::string& id, const std::string& name, std::shared_ptr<EditorPanel> panel)
	{
		TabData tab_data;
		tab_data.id = id;
		tab_data.name = name;
		tab_data.panel = panel;
		tab_data.is_visible = true;

		m_tabs.push_back(tab_data);

		// If this is the first tab, make it active
		if (m_tabs.size() == 1)
		{
			m_active_tab_index = 0;
		}
	}

	void TabbedPanelContainer::RemoveTab(const std::string& id)
	{
		for (size_t i = 0; i < m_tabs.size(); ++i)
		{
			if (m_tabs[i].id == id)
			{
				m_tabs.erase(m_tabs.begin() + i);

				// Adjust active tab index if needed
				if (m_active_tab_index >= static_cast<int>(m_tabs.size()))
				{
					m_active_tab_index = static_cast<int>(m_tabs.size()) - 1;
				}
				if (m_active_tab_index < 0)
				{
					m_active_tab_index = 0;
				}

				break;
			}
		}
	}

	void TabbedPanelContainer::SetActiveTab(int index)
	{
		if (index >= 0 && index < static_cast<int>(m_tabs.size()))
		{
			m_active_tab_index = index;
		}
	}

	void TabbedPanelContainer::SetActiveTabByID(const std::string& id)
	{
		for (size_t i = 0; i < m_tabs.size(); ++i)
		{
			if (m_tabs[i].id == id)
			{
				m_active_tab_index = static_cast<int>(i);
				break;
			}
		}
	}

	std::shared_ptr<EditorPanel> TabbedPanelContainer::GetActivePanel() const
	{
		if (m_active_tab_index >= 0 && m_active_tab_index < static_cast<int>(m_tabs.size()))
		{
			return m_tabs[m_active_tab_index].panel;
		}
		return nullptr;
	}

	void TabbedPanelContainer::OnImGuiRender()
	{
		// Fixed position at bottom of screen
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos;
		ImVec2 work_size = viewport->WorkSize;

		float container_height = 200.0f;
		float properties_panel_width = 300.0f;

		// Position: bottom, width = window width - properties panel width
		ImGui::SetNextWindowPos(ImVec2(work_pos.x, work_pos.y + work_size.y - container_height));
		ImGui::SetNextWindowSize(ImVec2(work_size.x - properties_panel_width, container_height));

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;

		if (ImGui::Begin("##BottomPanel", nullptr, window_flags))
		{
			RenderTabBar();
			RenderActivePanel();
		}
		ImGui::End();
	}

	void TabbedPanelContainer::RenderTabBar()
	{
		if (m_tabs.empty())
			return;

		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("BottomPanelTabs", tab_bar_flags))
		{
			for (size_t i = 0; i < m_tabs.size(); ++i)
			{
				const auto& tab = m_tabs[i];
				if (!tab.is_visible)
					continue;

				ImGuiTabItemFlags tab_flags = ImGuiTabItemFlags_None;
				bool is_selected = (static_cast<int>(i) == m_active_tab_index);

				if (ImGui::BeginTabItem(tab.name.c_str(), nullptr, tab_flags))
				{
					// Tab was clicked, update active index
					m_active_tab_index = static_cast<int>(i);
					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}
	}

	void TabbedPanelContainer::RenderActivePanel()
	{
		auto active_panel = GetActivePanel();
		if (active_panel)
		{
			// Render the active panel's content in a child window to contain it
			ImGui::BeginChild("ActivePanelContent", ImVec2(0, 0), false, ImGuiWindowFlags_None);
			active_panel->OnImGuiRender();
			ImGui::EndChild();
		}
	}

	void TabbedPanelContainer::OnEvent(EventBase& event)
	{
		// Forward events to all tabs (they can decide if they want to handle them)
		for (auto& tab : m_tabs)
		{
			if (tab.panel)
			{
				tab.panel->OnEvent(event);
			}
		}
	}

	void TabbedPanelContainer::SetSceneContext(class Scene* scene)
	{
		// Forward scene context to all tabs
		for (auto& tab : m_tabs)
		{
			if (tab.panel)
			{
				tab.panel->SetSceneContext(scene);
			}
		}
	}

} // namespace ignis
