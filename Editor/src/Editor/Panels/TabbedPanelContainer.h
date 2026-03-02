#pragma once

#include "Editor/Panels/EditorPanel.h"

namespace ignis {

	// Data for a single tab in the container
	struct TabData
	{
		std::string id;
		std::string name;
		std::shared_ptr<EditorPanel> panel;
		bool is_visible = true;
	};

	// Container that manages multiple panels with a tab bar interface
	// Allows switching between different panels in the same window area
	class TabbedPanelContainer : public EditorPanel
	{
	public:
		TabbedPanelContainer() = default;
		~TabbedPanelContainer() = default;

		// Add a new tab to the container
		void AddTab(const std::string& id, const std::string& name, std::shared_ptr<EditorPanel> panel);

		// Remove a tab by ID
		void RemoveTab(const std::string& id);

		// Set the active tab by index
		void SetActiveTab(int index);

		// Set the active tab by ID
		void SetActiveTabByID(const std::string& id);

		// Get the currently active panel
		std::shared_ptr<EditorPanel> GetActivePanel() const;

		// Get tab count
		size_t GetTabCount() const { return m_tabs.size(); }

		// EditorPanel interface implementation
		void OnImGuiRender() override;
		void OnEvent(EventBase& event) override;
		void SetSceneContext(class Scene* scene) override;
		std::string_view GetName() const override { return "Tabbed Panel Container"; }
		std::string_view GetID() const override { return "TabbedPanelContainer"; }

	private:
		void RenderTabBar();
		void RenderActivePanel();

	private:
		std::vector<TabData> m_tabs;
		int m_active_tab_index = 0;
	};

} // namespace ignis
