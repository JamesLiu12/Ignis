#pragma once

#include "EditorPanel.h"
#include <memory>
#include <vector>
#include <string>

namespace ignis {

	// Stores metadata and state for a single editor panel
	struct PanelData
	{
		std::string id;                          // Unique identifier
		std::string name;                        // Display name
		std::shared_ptr<EditorPanel> panel;      // The actual panel instance
		bool is_open = false;                    // Visibility state
	};

	// Manages the lifecycle and rendering of all editor panels
	class PanelManager
	{
	public:
		PanelManager() = default;
		~PanelManager() = default;

		// Add a panel to the manager
		template<typename TPanel, typename... TArgs>
		requires std::derived_from<TPanel, EditorPanel>
		std::shared_ptr<TPanel> AddPanel(const std::string& id, const std::string& name, bool is_open_by_default, TArgs&&... args)
		{
			// Create the panel
			auto panel_instance = std::make_shared<TPanel>(std::forward<TArgs>(args)...);

			// Store panel data
			PanelData data;
			data.id = id;
			data.name = name;
			data.panel = panel_instance;
			data.is_open = is_open_by_default;

			m_panels.push_back(data);

			return panel_instance;
		}

		// Get a panel by its ID
		template<typename TPanel>
		std::shared_ptr<TPanel> GetPanel(const std::string& id)
		{
			for (auto& panel_data : m_panels)
			{
				if (panel_data.id == id)
					return std::static_pointer_cast<TPanel>(panel_data.panel);
			}
			return nullptr;
		}

		// Render all open panels
		void OnImGuiRender();

		// Forward events to all panels
		void OnEvent(EventBase& e);

		// Set scene context for all panels
		void SetSceneContext(class Scene* scene);

		// Get all panels for menu/UI display
		std::vector<PanelData>& GetPanels() { return m_panels; }
		const std::vector<PanelData>& GetPanels() const { return m_panels; }

	private:
		std::vector<PanelData> m_panels;
	};

} // namespace ignis