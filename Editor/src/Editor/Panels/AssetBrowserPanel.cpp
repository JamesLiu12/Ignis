#include "AssetBrowserPanel.h"
#include "Ignis/Asset/AssetManager.h"
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
		if (ImGui::Button("^"))
		{
			NavigateToParent();
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Up to Parent");
		
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
	
	void AssetBrowserPanel::Refresh()
	{
		if (Project::GetActive())
		{
			InitializeFromProject();
		}
		else
		{
			Log::Warn("No active project to refresh");
		}
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

} // namespace ignis
