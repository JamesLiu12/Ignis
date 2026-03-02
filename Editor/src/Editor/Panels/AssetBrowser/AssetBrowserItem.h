#pragma once

#include "Ignis/Asset/AssetManager.h"

namespace ignis {

	// Forward declarations
	class AssetBrowserPanel;

	// Represents a single item in the asset browser
	class AssetBrowserItem
	{
	public:
		enum class ItemType : uint16_t
		{
			Directory,
			Asset
		};

	public:
		AssetBrowserItem(ItemType type, AssetHandle id, const std::string& name);
		virtual ~AssetBrowserItem() = default;

		// Render the item in the asset UI
		void OnRender();
		
		// Called when item is activated (double-clicked)
		virtual void OnActivate() {}
		
		// Set the owning panel (for navigation callbacks)
		void SetPanel(AssetBrowserPanel* panel) { m_panel = panel; }

		// Delete
		virtual void Delete() {}

		// Move to new location
		virtual bool Move(const std::filesystem::path& destination) { return false; }

		// Getters
		AssetHandle GetID() const { return m_id; }
		ItemType GetType() const { return m_type; }
		const std::string& GetName() const { return m_file_name; }
		const std::string& GetDisplayName() const { return m_display_name; }

		// Renaming
		void StartRenaming();
		void StopRenaming();
		bool IsRenaming() const { return m_is_renaming; }
		void Rename(const std::string& new_name);

		// Selection
		bool IsSelected() const { return m_is_selected; }
		void SetSelected(bool selected) { m_is_selected = selected; }

	protected:
		virtual void OnRenamed(const std::string& new_name) { m_file_name = new_name; }
		void SetDisplayNameFromFileName();

	protected:
		ItemType m_type;
		AssetHandle m_id;
		std::string m_display_name;
		std::string m_file_name;

		bool m_is_renaming = false;
		bool m_is_selected = false;
		bool m_is_dragging = false;

		char m_rename_buffer[128];
		
		AssetBrowserPanel* m_panel = nullptr;
	};

	// Represents directory information
	struct DirectoryInfo
	{
		AssetHandle handle;
		std::shared_ptr<DirectoryInfo> parent;
		std::filesystem::path file_path;
		std::vector<AssetHandle> assets;
		std::map<AssetHandle, std::shared_ptr<DirectoryInfo>> sub_directories;
	};

	// Represents a directory item
	class AssetBrowserDirectory : public AssetBrowserItem
	{
	public:
		AssetBrowserDirectory(const std::shared_ptr<DirectoryInfo>& directory_info);
		virtual ~AssetBrowserDirectory() = default;

		std::shared_ptr<DirectoryInfo>& GetDirectoryInfo() { return m_directory_info; }

		void OnActivate() override;
		void Delete() override;
		bool Move(const std::filesystem::path& destination) override;

	protected:
		void OnRenamed(const std::string& new_name) override;

	private:
		std::shared_ptr<DirectoryInfo> m_directory_info;
	};

	// Represents an asset item
	class AssetBrowserAsset : public AssetBrowserItem
	{
	public:
		AssetBrowserAsset(const AssetMetadata& asset_info);
		virtual ~AssetBrowserAsset() = default;

		const AssetMetadata& GetAssetInfo() const { return m_asset_info; }

		void Delete() override;
		bool Move(const std::filesystem::path& destination) override;

	protected:
		void OnRenamed(const std::string& new_name) override;

	private:
		AssetMetadata m_asset_info;
	};

} // namespace ignis
