#pragma once

#include "Editor/Panels/EditorPanel.h"
#include "Editor/Panels/AssetBrowser/AssetBrowserItem.h"
#include "Ignis/Project/Project.h"
#include <imgui.h>

namespace ignis {

	class AssetBrowserPanel : public EditorPanel
	{
	public:
		AssetBrowserPanel();
		~AssetBrowserPanel() = default;

		void OnImGuiRender() override;
		void OnEvent(EventBase& event) override {}
		void SetSceneContext(class Scene* scene) override {}
		std::string_view GetName() const override { return "Assets"; }
		std::string_view GetID() const override { return "AssetBrowser"; }

		// Get icon color for an item
		ImVec4 GetIconColor(const AssetBrowserItem& item) const;
		
		// Navigation (public for item callbacks)
		void ChangeDirectory(const std::shared_ptr<DirectoryInfo>& directory);
		
		// Project integration (public for external refresh)
		void Refresh();
		
		// Selection management (public for item callbacks)
		bool IsItemSelected(AssetBrowserItem* item) const;
		void SetSelectedItem(AssetBrowserItem* item);
		AssetBrowserItem* GetSelectedItem() const { return m_selected_item; }
		
		// File operations (public for item callbacks)
		void ShowInExplorer(const std::filesystem::path& path);

	private:
		void RenderTopBar();
		void RenderBreadcrumbs();
		void RenderItems();
		
		// Directory processing
		void LoadCurrentDirectory();
		std::shared_ptr<DirectoryInfo> ProcessDirectory(const std::filesystem::path& directory_path, const std::shared_ptr<DirectoryInfo>& parent);
		std::shared_ptr<DirectoryInfo> FindDirectoryByPath(const std::shared_ptr<DirectoryInfo>& root, const std::filesystem::path& target_path);
		
		// Navigation
		void NavigateToParent();
		void NavigateBack();
		void NavigateForward();
		
		// Project integration
		void InitializeFromProject();
		
		// File Creation
		void HandleKeyboardInput();
		void CreateNewFolder();
		void CreateNewFile();

	private:
		std::vector<std::shared_ptr<AssetBrowserItem>> m_current_items;
		std::shared_ptr<DirectoryInfo> m_current_directory;
		std::shared_ptr<DirectoryInfo> m_base_directory;
		
		// Navigation history
		std::vector<std::shared_ptr<DirectoryInfo>> m_backward_history;
		std::vector<std::shared_ptr<DirectoryInfo>> m_forward_history;
		
		// Breadcrumb data
		std::vector<std::shared_ptr<DirectoryInfo>> m_breadcrumb_data;

		// Icon colors
		ImVec4 m_folder_color = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
		ImVec4 m_file_color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

		// Grid layout settings
		float m_thumbnail_size = 64.0f;
		float m_padding = 16.0f;
		
		// Selection management
		AssetBrowserItem* m_selected_item = nullptr;
		
		// State flags
		bool m_update_breadcrumbs = false;
	};

} // namespace ignis
