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

	private:
		void RenderTopBar();
		void RenderItems();
		void LoadCurrentDirectory();
		void CreateTestItems();

	private:
		std::vector<std::shared_ptr<AssetBrowserItem>> m_current_items;
		std::shared_ptr<DirectoryInfo> m_current_directory;
		std::shared_ptr<DirectoryInfo> m_base_directory;

		// Icon colors
		ImVec4 m_folder_color = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
		ImVec4 m_file_color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

		// Grid layout settings
		float m_thumbnail_size = 64.0f;
		float m_padding = 16.0f;
	};

} // namespace ignis
