#include "AssetBrowserPanel.h"
#include <imgui.h>

namespace ignis {

	AssetBrowserPanel::AssetBrowserPanel()
	{
		// Create test items for demonstration
		CreateTestItems();
	}

	void AssetBrowserPanel::OnImGuiRender()
	{
		RenderTopBar();
		ImGui::Separator();
		RenderItems();
	}

	ImVec4 AssetBrowserPanel::GetIconColor(const AssetBrowserItem& item) const
	{
		if (item.GetType() == AssetBrowserItem::ItemType::Directory)
			return m_folder_color;
		else
			return m_file_color;
	}

	void AssetBrowserPanel::RenderTopBar()
	{
		// Top bar with title
		ImGui::Text("Asset Browser");
	}

	void AssetBrowserPanel::RenderItems()
	{
		// Calculate grid layout
		float panel_width = ImGui::GetContentRegionAvail().x;
		float cell_size = m_thumbnail_size + m_padding;
		int columns = std::max(1, static_cast<int>(panel_width / cell_size));

		// Render items in grid
		ImGui::Columns(columns, nullptr, false);

		for (auto& item : m_current_items)
		{
			item->OnRender();
			ImGui::NextColumn();
		}

		ImGui::Columns(1);
	}

	void AssetBrowserPanel::LoadCurrentDirectory()
	{
		// TODO: Load actual directory contents from project
	}

	void AssetBrowserPanel::CreateTestItems()
	{
		// Create some test items
		m_current_items.clear();

		// Create test directories
		auto dir1 = std::make_shared<DirectoryInfo>();
		dir1->handle = AssetHandle();
		dir1->file_path = "models";
		m_current_items.push_back(std::make_shared<AssetBrowserDirectory>(dir1));

		auto dir2 = std::make_shared<DirectoryInfo>();
		dir2->handle = AssetHandle();
		dir2->file_path = "textures";
		m_current_items.push_back(std::make_shared<AssetBrowserDirectory>(dir2));

		auto dir3 = std::make_shared<DirectoryInfo>();
		dir3->handle = AssetHandle();
		dir3->file_path = "shaders";
		m_current_items.push_back(std::make_shared<AssetBrowserDirectory>(dir3));

		auto dir4 = std::make_shared<DirectoryInfo>();
		dir4->handle = AssetHandle();
		dir4->file_path = "scenes";
		m_current_items.push_back(std::make_shared<AssetBrowserDirectory>(dir4));

		// Create test asset files
		AssetMetadata asset1;
		asset1.Handle = AssetHandle();
		asset1.FilePath = "gun.fbx";
		asset1.Type = AssetType::Mesh;
		m_current_items.push_back(std::make_shared<AssetBrowserAsset>(asset1));

		AssetMetadata asset2;
		asset2.Handle = AssetHandle();
		asset2.FilePath = "wall.png";
		asset2.Type = AssetType::Texture2D;
		m_current_items.push_back(std::make_shared<AssetBrowserAsset>(asset2));

		Log::Info("Created {} test items for Asset Browser", m_current_items.size());
	}

} // namespace ignis
