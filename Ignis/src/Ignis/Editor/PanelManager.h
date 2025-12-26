#pragma once

#include "EditorPanel.h"
#include <memory>
#include <vector>
#include <string>

namespace ignis {

	/// <summary>
	/// Stores metadata and state for a single editor panel
	/// </summary>
	struct PanelData
	{
		std::string ID;                          // Unique identifier
		std::string Name;                        // Display name
		std::shared_ptr<EditorPanel> Panel;      // The actual panel instance
		bool IsOpen = false;                     // Visibility state
	};

	/// <summary>
	/// Manages the lifecycle and rendering of all editor panels
	/// Simplified version without docking/serialization
	/// </summary>
	class PanelManager
	{
	public:
		PanelManager() = default;
		~PanelManager() = default;

		/// <summary>
		/// Add a panel to the manager
		/// </summary>
		template<typename TPanel, typename... TArgs>
		requires std::derived_from<TPanel, EditorPanel>
		std::shared_ptr<TPanel> AddPanel(const std::string& id, const std::string& name, bool isOpenByDefault, TArgs&&... args)
		{
			// Create the panel
			auto panel = std::make_shared<TPanel>(std::forward<TArgs>(args)...);

			// Store panel data
			PanelData data;
			data.ID = id;
			data.Name = name;
			data.Panel = panel;
			data.IsOpen = isOpenByDefault;

			m_panels.push_back(data);

			return panel;
		}

		/// <summary>
		/// Get a panel by its ID
		/// </summary>
		template<typename TPanel>
		std::shared_ptr<TPanel> GetPanel(const std::string& id)
		{
			for (auto& panelData : m_panels)
			{
				if (panelData.ID == id)
					return std::static_pointer_cast<TPanel>(panelData.Panel);
			}
			return nullptr;
		}

		/// <summary>
		/// Render all open panels
		/// </summary>
		void OnImGuiRender();

		/// <summary>
		/// Forward events to all panels
		/// </summary>
		void OnEvent(EventBase& e);

		/// <summary>
		/// Set scene context for all panels
		/// </summary>
		void SetSceneContext(class Scene* scene);

		/// <summary>
		/// Get all panels for menu/UI display
		/// </summary>
		std::vector<PanelData>& GetPanels() { return m_panels; }
		const std::vector<PanelData>& GetPanels() const { return m_panels; }

	private:
		std::vector<PanelData> m_panels;
	};

} // namespace ignis