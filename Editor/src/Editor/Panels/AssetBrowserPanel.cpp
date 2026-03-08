#include "AssetBrowserPanel.h"
#include "Ignis/Asset/AssetManager.h"
#include "Editor/Panels/PropertiesPanel.h"
#include <imgui.h>

namespace ignis {

	AssetBrowserPanel::AssetBrowserPanel()
	{
		// Initialize from active project if available
		if (Project::GetActive())
		{
			InitializeFromProject();
		}
		else
		{
			Log::Info("No active project loaded - Asset Browser is empty");
		}
	}

	void AssetBrowserPanel::OnImGuiRender()
	{
		RenderTopBar();
		ImGui::Separator();
		RenderBreadcrumbs();
		ImGui::Separator();
		RenderItems();
		
		// Handle keyboard input
		HandleKeyboardInput();
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
		// Navigation buttons
		if (ImGui::Button("<"))
		{
			NavigateBack();
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Back");
		
		ImGui::SameLine();
		if (ImGui::Button(">"))
		{
			NavigateForward();
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Forward");
		
		ImGui::SameLine();
		if (ImGui::Button("Refresh"))
		{
			Refresh();
		}
	}

	void AssetBrowserPanel::RenderItems()
	{
		// Add scrolling region for many files
		ImGui::BeginChild("AssetGrid", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		
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
		
		// Right-click on empty space for context menu
		if (ImGui::BeginPopupContextWindow("empty_context", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create New Folder"))
			{
				CreateNewFolder();
			}
			if (ImGui::MenuItem("Create New File"))
			{
				CreateNewFile();
			}
			ImGui::EndPopup();
		}
		
		ImGui::EndChild();
	}

	void AssetBrowserPanel::RenderBreadcrumbs()
	{
		if (!m_current_directory)
			return;
		
		// Build breadcrumb path if needed
		if (m_update_breadcrumbs)
		{
			m_breadcrumb_data.clear();
			auto dir = m_current_directory;
			while (dir)
			{
				m_breadcrumb_data.insert(m_breadcrumb_data.begin(), dir);
				dir = dir->parent;
			}
			m_update_breadcrumbs = false;
		}
		
		// Render breadcrumbs
		for (size_t i = 0; i < m_breadcrumb_data.size(); i++)
		{
			if (i > 0)
			{
				ImGui::SameLine();
				ImGui::Text("/");
				ImGui::SameLine();
			}
			
			std::string name = m_breadcrumb_data[i]->file_path.filename().string();
			if (name.empty())
				name = "Assets";
			
			if (ImGui::Button(name.c_str()))
			{
				ChangeDirectory(m_breadcrumb_data[i]);
			}
		}
	}
	
	void AssetBrowserPanel::LoadCurrentDirectory()
	{
		if (!m_current_directory)
			return;
		
		m_current_items.clear();
		
		// Add subdirectories
		for (const auto& [handle, subdir] : m_current_directory->sub_directories)
		{
			auto item = std::make_shared<AssetBrowserDirectory>(subdir);
			item->SetPanel(this);
			m_current_items.push_back(item);
		}
		
		// Add assets
		for (const auto& asset_handle : m_current_directory->assets)
		{
			const AssetMetadata* metadata = AssetManager::GetMetadata(asset_handle);
			if (metadata)
			{
				auto item = std::make_shared<AssetBrowserAsset>(*metadata);
				item->SetPanel(this);
				m_current_items.push_back(item);
			}
		}
		
		// Sort: directories first, then files, alphabetically
		std::sort(m_current_items.begin(), m_current_items.end(),
			[](const std::shared_ptr<AssetBrowserItem>& a, const std::shared_ptr<AssetBrowserItem>& b)
			{
				if (a->GetType() != b->GetType())
					return a->GetType() == AssetBrowserItem::ItemType::Directory;
				return a->GetName() < b->GetName();
			});
	}

	std::shared_ptr<DirectoryInfo> AssetBrowserPanel::ProcessDirectory(const std::filesystem::path& directory_path, const std::shared_ptr<DirectoryInfo>& parent)
	{
		auto directory_info = std::make_shared<DirectoryInfo>();
		directory_info->handle = AssetHandle();
		directory_info->parent = parent;
		directory_info->file_path = directory_path;
		
		if (!std::filesystem::exists(directory_path))
		{
			Log::Warn("Directory does not exist: {}", directory_path.string());
			return directory_info;
		}
		
		// Iterate through directory contents
		for (const auto& entry : std::filesystem::directory_iterator(directory_path))
		{
			if (entry.is_directory())
			{
				// Recursively process subdirectory
				auto subdir = ProcessDirectory(entry.path(), directory_info);
				directory_info->sub_directories[subdir->handle] = subdir;
			}
			else if (entry.is_regular_file())
			{
				// Check if this file is in the asset registry
				const AssetMetadata* metadata = AssetManager::GetMetadata(entry.path());
				if (metadata)
				{
					directory_info->assets.push_back(metadata->Handle);
				}
				else
				{
					// Show unregistered files too (import them on-demand)
					AssetHandle handle = AssetManager::ImportAsset(entry.path());
					if (handle.IsValid())
					{
						directory_info->assets.push_back(handle);
					}
				}
			}
		}
		
		return directory_info;
	}
	
	std::shared_ptr<DirectoryInfo> AssetBrowserPanel::FindDirectoryByPath(const std::shared_ptr<DirectoryInfo>& root, const std::filesystem::path& target_path)
	{
		if (!root)
			return nullptr;
		
		if (root->file_path == target_path)
			return root;
		
		// Recursively search subdirectories
		for (const auto& [handle, subdir] : root->sub_directories)
		{
			auto found = FindDirectoryByPath(subdir, target_path);
			if (found)
				return found;
		}
		
		return nullptr;
	}
	
	void AssetBrowserPanel::ChangeDirectory(const std::shared_ptr<DirectoryInfo>& directory)
	{
		if (!directory)
			return;
		
		// Add current directory to backward history
		if (m_current_directory)
		{
			m_backward_history.push_back(m_current_directory);
		}
		
		// Clear forward history when navigating to a new directory
		m_forward_history.clear();
		
		// Clear single selection when changing directories
		m_selected_item = nullptr;
		
		m_current_directory = directory;
		m_update_breadcrumbs = true;
		LoadCurrentDirectory();
	}
	
	void AssetBrowserPanel::NavigateToParent()
	{
		if (m_current_directory && m_current_directory->parent)
		{
			ChangeDirectory(m_current_directory->parent);
		}
	}
	
	void AssetBrowserPanel::NavigateBack()
	{
		if (m_backward_history.empty())
			return;
		
		// Move current to forward history
		if (m_current_directory)
		{
			m_forward_history.push_back(m_current_directory);
		}
		
		// Pop from backward history
		m_current_directory = m_backward_history.back();
		m_backward_history.pop_back();
		
		m_update_breadcrumbs = true;
		LoadCurrentDirectory();
	}
	
	void AssetBrowserPanel::NavigateForward()
	{
		if (m_forward_history.empty())
			return;
		
		// Move current to backward history
		if (m_current_directory)
		{
			m_backward_history.push_back(m_current_directory);
		}
		
		// Pop from forward history
		m_current_directory = m_forward_history.back();
		m_forward_history.pop_back();
		
		m_update_breadcrumbs = true;
		LoadCurrentDirectory();
	}
	
	void AssetBrowserPanel::Clear()
	{
		// Clear all state
		m_current_items.clear();
		m_current_directory = nullptr;
		m_base_directory = nullptr;
		m_backward_history.clear();
		m_forward_history.clear();
		m_breadcrumb_data.clear();
		m_selected_item = nullptr;
		m_update_breadcrumbs = false;
		
		Log::Info("Asset Browser cleared");
	}
	
	void AssetBrowserPanel::Refresh()
	{
		if (!Project::GetActive())
		{
			Log::Warn("No active project to refresh");
			return;
		}
		
		// Save current directory path before refresh
		std::filesystem::path current_path;
		if (m_current_directory)
			current_path = m_current_directory->file_path;
		
		// Rebuild entire tree from filesystem
		std::filesystem::path asset_directory = Project::GetActiveAssetDirectory();
		
		if (!std::filesystem::exists(asset_directory))
		{
			Log::Error("Asset directory does not exist: {}", asset_directory.string());
			return;
		}
		
		Log::Info("Refreshing Asset Browser from: {}", asset_directory.string());
		
		m_base_directory = ProcessDirectory(asset_directory, nullptr);
		
		// Navigate back to saved path if it still exists
		if (!current_path.empty() && std::filesystem::exists(current_path))
		{
			// Find directory in new tree
			m_current_directory = FindDirectoryByPath(m_base_directory, current_path);
			if (!m_current_directory)
			{
				// Fallback to root if path not found in tree
				m_current_directory = m_base_directory;
			}
		}
		else
		{
			// No saved path or path doesn't exist, go to root
			m_current_directory = m_base_directory;
		}
		
		// Clear navigation history since tree was rebuilt
		m_backward_history.clear();
		m_forward_history.clear();
		
		m_update_breadcrumbs = true;
		LoadCurrentDirectory();
		
		Log::Info("Asset Browser refreshed with {} items", m_current_items.size());
	}
	
	void AssetBrowserPanel::InitializeFromProject()
	{
		auto project = Project::GetActive();
		if (!project)
		{
			Log::Error("Cannot initialize Asset Browser: No active project");
			return;
		}
		
		std::filesystem::path asset_directory = Project::GetActiveAssetDirectory();
		
		if (!std::filesystem::exists(asset_directory))
		{
			Log::Error("Asset directory does not exist: {}", asset_directory.string());
			return;
		}
		
		Log::Info("Initializing Asset Browser from project: {}", asset_directory.string());
		
		// Process the entire asset directory tree
		m_base_directory = ProcessDirectory(asset_directory, nullptr);
		m_current_directory = m_base_directory;
		
		// Clear navigation history
		m_backward_history.clear();
		m_forward_history.clear();
		
		m_update_breadcrumbs = true;
		LoadCurrentDirectory();
		
		Log::Info("Asset Browser initialized with {} items", m_current_items.size());
	}
	
	// Selection management
	bool AssetBrowserPanel::IsItemSelected(AssetBrowserItem* item) const
	{
		return m_selected_item == item;
	}
	
	void AssetBrowserPanel::SetSelectedItem(AssetBrowserItem* item)
	{
		m_selected_item = item;
		
		// Notify Properties Panel if an asset is selected
		if (item && item->GetType() == AssetBrowserItem::ItemType::Asset)
		{
			auto* asset_item = static_cast<AssetBrowserAsset*>(item);
			AssetHandle handle = asset_item->GetAssetInfo().Handle;
			
			// Update Properties Panel directly
			if (m_properties_panel)
			{
				m_properties_panel->SetSelectedAsset(handle);
			}
		}
	}
	
	// Keyboard input handling
	void AssetBrowserPanel::HandleKeyboardInput()
	{
		// Only handle input if panel is focused
		if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows))
			return;
		
		// Check if any item is currently being renamed
		bool any_renaming = false;
		for (const auto& item : m_current_items)
		{
			if (item->IsRenaming())
			{
				any_renaming = true;
				break;
			}
		}
		
		// Don't process keyboard shortcuts if an item is being renamed
		// (except Escape to cancel rename)
		if (any_renaming)
		{
			// Escape - Cancel rename
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				for (const auto& item : m_current_items)
				{
					if (item->IsRenaming())
					{
						item->StopRenaming();
					}
				}
			}
			return; // Don't process other shortcuts during rename
		}
		
		// F2 - Rename selected item
		if (ImGui::IsKeyPressed(ImGuiKey_F2) && m_selected_item)
		{
			m_selected_item->StartRenaming();
		}
		
		// Delete/Backspace - Delete selected item
		if ((ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace)) && m_selected_item)
		{
			m_selected_item->Delete();
			m_selected_item = nullptr;
		}
		
		// Escape - Clear selection
		if (ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			m_selected_item = nullptr;
		}
	}
	
	// Create new folder
	void AssetBrowserPanel::CreateNewFolder()
	{
		if (!m_current_directory)
			return;
		
		std::filesystem::path new_folder_path = m_current_directory->file_path / "New Folder";
		
		// Find unique name if "New Folder" already exists
		int counter = 1;
		while (std::filesystem::exists(new_folder_path))
		{
			new_folder_path = m_current_directory->file_path / ("New Folder " + std::to_string(counter));
			counter++;
		}
		
		try
		{
			std::filesystem::create_directory(new_folder_path);
			Log::Info("Created new folder: {}", new_folder_path.string());
			
			// Re-scan current directory from filesystem to update tree
			auto updated_dir = ProcessDirectory(m_current_directory->file_path, m_current_directory->parent);
			
			// Update parent's reference to this directory
			if (m_current_directory->parent)
			{
				m_current_directory->parent->sub_directories[updated_dir->handle] = updated_dir;
			}
			else
			{
				// Root directory
				m_base_directory = updated_dir;
			}
			
			m_current_directory = updated_dir;
			LoadCurrentDirectory();
			
			// Find and start renaming the new folder
			for (auto& item : m_current_items)
			{
				if (item->GetType() == AssetBrowserItem::ItemType::Directory)
				{
					if (item->GetName() == new_folder_path.filename().string())
					{
						item->StartRenaming();
						break;
					}
				}
			}
		}
		catch (const std::exception& e)
		{
			Log::Error("Failed to create folder: {}", e.what());
		}
	}
	
	void AssetBrowserPanel::CreateNewFile()
	{
		if (!m_current_directory)
			return;
		
		std::filesystem::path new_file_path = m_current_directory->file_path / "New File.txt";
		
		// Find unique name if "New File.txt" already exists
		int counter = 1;
		while (std::filesystem::exists(new_file_path))
		{
			new_file_path = m_current_directory->file_path / ("New File " + std::to_string(counter) + ".txt");
			counter++;
		}
		
		try
		{
			// Create empty file
			std::ofstream file(new_file_path);
			file.close();
			Log::Info("Created new file: {}", new_file_path.string());
			
			// Import the new file as an asset
			AssetHandle new_handle = AssetManager::ImportAsset(new_file_path, AssetType::Unknown);
			if (new_handle.IsValid())
			{
				AssetManager::SaveAssetRegistry(Project::GetActiveAssetRegistry());
				Log::Info("Registered new file as asset: {}", new_file_path.string());
			}
			
			// Refresh to show new file
			Refresh();
			
			// Find and start renaming the new file
			for (auto& item : m_current_items)
			{
				if (item->GetType() == AssetBrowserItem::ItemType::Asset)
				{
					if (item->GetName() == new_file_path.filename().string())
					{
						item->StartRenaming();
						break;
					}
				}
			}
		}
		catch (const std::exception& e)
		{
			Log::Error("Failed to create file: {}", e.what());
		}
	}
	
	// Show in Explorer (macOS Finder)
	void AssetBrowserPanel::ShowInExplorer(const std::filesystem::path& path)
	{
		#ifdef __APPLE__
			std::string command = "open -R \"" + path.string() + "\"";
			system(command.c_str());
		#elif _WIN32
			std::string command = "explorer /select,\"" + path.string() + "\"";
			system(command.c_str());
		#else
			// Linux - open parent directory
			std::string command = "xdg-open \"" + path.parent_path().string() + "\"";
			system(command.c_str());
		#endif
	}

} // namespace ignis
