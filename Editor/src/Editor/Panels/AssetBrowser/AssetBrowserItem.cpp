#include "AssetBrowserItem.h"
#include "Editor/Panels/AssetBrowserPanel.h"
#include <imgui.h>

#ifdef _WIN32
#include <Windows.h>
#include <shellapi.h>
#endif

namespace ignis {

	AssetBrowserItem::AssetBrowserItem(ItemType type, AssetHandle id, const std::string& name)
		: m_type(type), m_id(id), m_file_name(name)
	{
		SetDisplayNameFromFileName();
		std::memset(m_rename_buffer, 0, sizeof(m_rename_buffer));
	}

	void AssetBrowserItem::OnRender()
	{
		// Determine icon color
		ImVec4 icon_color;
		if (m_type == ItemType::Directory)
			icon_color = ImVec4(0.2f, 0.6f, 1.0f, 1.0f);
		else
			icon_color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);

		// Create unique ID for this item using pointer address
		ImGui::PushID(this);

		// Start group for the item
		ImGui::BeginGroup();

		// Render icon as colored square (64x64)
		ImGui::ColorButton("##icon", icon_color, ImGuiColorEditFlags_NoTooltip, ImVec2(64, 64));

		// Render name
		if (m_is_renaming)
		{
			ImGui::SetKeyboardFocusHere();
			if (ImGui::InputText("##rename", m_rename_buffer, sizeof(m_rename_buffer), 
			                     ImGuiInputTextFlags_EnterReturnsTrue))
			{
				Rename(m_rename_buffer);
				StopRenaming();
			}

			// Cancel rename on Escape
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				StopRenaming();
			}
		}
		else
		{
			// Display name with selection highlight
			bool is_selected = m_panel ? m_panel->IsItemSelected(this) : m_is_selected;
			if (is_selected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow for selected
			}

			ImGui::TextWrapped("%s", m_display_name.c_str());

			if (is_selected)
			{
				ImGui::PopStyleColor();
			}
		}

		ImGui::EndGroup();

		// Handle double click on the entire group (avoid single click interference)
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			OnActivate();
		}
		// Handle single click selection
		else if (ImGui::IsItemClicked())
		{
			if (m_panel)
			{
				// Simple single selection
				m_panel->SetSelectedItem(this);
			}
			else
			{
				// Fallback if no panel
				m_is_selected = true;
			}
		}

		// Handle right-click context menu
		if (ImGui::BeginPopupContextItem("item_context"))
		{
			if (ImGui::MenuItem("Rename", "F2"))
			{
				StartRenaming();
			}
		
			if (ImGui::MenuItem("Delete", "Del"))
			{
				Delete();
			}
		
			if (ImGui::MenuItem("Show in Explorer"))
			{
				ShowInExplorer();
			}
		
			ImGui::EndPopup();
		}

		ImGui::PopID();
	}

	void AssetBrowserItem::StartRenaming()
	{
		m_is_renaming = true;
		std::strncpy(m_rename_buffer, m_file_name.c_str(), sizeof(m_rename_buffer) - 1);
		m_rename_buffer[sizeof(m_rename_buffer) - 1] = '\0';
	}

	void AssetBrowserItem::StopRenaming()
	{
		m_is_renaming = false;
		std::memset(m_rename_buffer, 0, sizeof(m_rename_buffer));
	}

	void AssetBrowserItem::Rename(const std::string& new_name)
	{
		if (new_name.empty() || new_name == m_file_name)
			return;

		OnRenamed(new_name);
		SetDisplayNameFromFileName();
	}

	void AssetBrowserItem::SetDisplayNameFromFileName()
	{
		m_display_name = m_file_name;
	}

	void AssetBrowserItem::ShowInExplorer()
	{
		// Delegate to panel if available
		if (m_panel)
		{
			// Get full path from derived class
			std::filesystem::path full_path;
			if (m_type == ItemType::Directory)
			{
				auto* dir = dynamic_cast<AssetBrowserDirectory*>(this);
				if (dir)
					full_path = dir->GetDirectoryInfo()->file_path;
			}
			else
			{
				auto* asset = dynamic_cast<AssetBrowserAsset*>(this);
				if (asset)
				{
					const AssetMetadata& metadata = asset->GetAssetInfo();
					full_path = Project::GetActiveAssetDirectory() / metadata.FilePath;
				}
			}
			
			if (!full_path.empty())
			{
				m_panel->ShowInExplorer(full_path);
			}
		}
	}

	// AssetBrowserDirectory implementation
	AssetBrowserDirectory::AssetBrowserDirectory(const std::shared_ptr<DirectoryInfo>& directory_info)
		: AssetBrowserItem(ItemType::Directory, directory_info->handle, directory_info->file_path.filename().string())
		, m_directory_info(directory_info)
	{
	}

	void AssetBrowserDirectory::OnActivate()
	{
		if (m_panel)
		{
			m_panel->ChangeDirectory(m_directory_info);
		}
	}
	
	void AssetBrowserDirectory::Delete()
	{
		std::filesystem::path path_to_delete = m_directory_info->file_path;
		
		if (!std::filesystem::exists(path_to_delete))
		{
			Log::Error("Directory does not exist: {}", path_to_delete.string());
			return;
		}
		
		try
		{
			bool deleted_successfully = false;
			
			// Move to Trash instead of permanent deletion
			#ifdef __APPLE__
				// macOS: Use osascript to move to Trash
				std::string command = "osascript -e 'tell application \"Finder\" to delete POSIX file \"" + path_to_delete.string() + "\"'";
				int result = system(command.c_str());
				if (result == 0)
				{
					Log::Info("Moved directory to Trash: {}", path_to_delete.string());
					deleted_successfully = true;
				}
				else
				{
					Log::Error("Failed to move directory to Trash. Please delete via Finder.");
				}
			#elif _WIN32
				// Windows: Use Shell API for fast Recycle Bin operation
				std::string path_str = path_to_delete.string();
				// SHFileOperation requires double-null terminated string
				std::vector<char> double_null_path(path_str.begin(), path_str.end());
				double_null_path.push_back('\0');
				double_null_path.push_back('\0');
				
				SHFILEOPSTRUCTA file_op = {};
				file_op.wFunc = FO_DELETE;
				file_op.pFrom = double_null_path.data();
				file_op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
				
				int result = SHFileOperationA(&file_op);
				if (result == 0 && !file_op.fAnyOperationsAborted)
				{
					Log::Info("Moved directory to Recycle Bin: {}", path_to_delete.string());
					deleted_successfully = true;
				}
				else
				{
					Log::Error("Failed to move directory to Recycle Bin. Error code: {}", result);
				}
			#else
				// Linux: Try multiple trash methods
				std::string command = "gio trash \"" + path_to_delete.string() + "\" 2>/dev/null";
				int result = system(command.c_str());
				if (result == 0)
				{
					Log::Info("Moved directory to trash: {}", path_to_delete.string());
					deleted_successfully = true;
				}
				else
				{
					// Try trash-cli as fallback
					command = "trash-put \"" + path_to_delete.string() + "\" 2>/dev/null";
					result = system(command.c_str());
					if (result == 0)
					{
						Log::Info("Moved directory to trash: {}", path_to_delete.string());
						deleted_successfully = true;
					}
					else
					{
						Log::Error("Cannot delete directory: trash functionality not available. Please delete via file manager or install 'gio' or 'trash-cli'.");
					}
				}
			#endif
			
			// Only refresh if deletion was successful
			if (deleted_successfully && m_panel)
			{
				m_panel->Refresh();
			}
		}
		catch (const std::exception& e)
		{
			Log::Error("Failed to delete directory: {}", e.what());
		}
	}

	void AssetBrowserDirectory::OnRenamed(const std::string& new_name)
	{
		if (new_name == m_file_name)
			return;
		
		std::filesystem::path old_path = m_directory_info->file_path;
		std::filesystem::path new_path = old_path.parent_path() / new_name;
		
		try
		{
			std::filesystem::rename(old_path, new_path);
			m_directory_info->file_path = new_path;
			m_file_name = new_name;
			Log::Info("Renamed directory: {} -> {}", old_path.string(), new_path.string());
		}
		catch (const std::exception& e)
		{
			Log::Error("Failed to rename directory: {}", e.what());
		}
	}

	// AssetBrowserAsset implementation
	AssetBrowserAsset::AssetBrowserAsset(const AssetMetadata& asset_info)
		: AssetBrowserItem(ItemType::Asset, asset_info.Handle, asset_info.FilePath.filename().string())
		, m_asset_info(asset_info)
	{
	}

	void AssetBrowserAsset::OnActivate()
	{
		// Open file with system default application
		std::filesystem::path full_path = Project::GetActiveAssetDirectory() / m_asset_info.FilePath;
		
		if (!std::filesystem::exists(full_path))
		{
			Log::Error("File does not exist: {}", full_path.string());
			return;
		}
		
		#ifdef __APPLE__
			// macOS: Try simple 'open' command first (works for files with default apps)
			std::string command = "open \"" + full_path.string() + "\" 2>&1";
			FILE* pipe = popen(command.c_str(), "r");
			bool opened_successfully = false;
			
			if (pipe)
			{
				char buffer[128];
				std::string result_str;
				while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
				{
					result_str += buffer;
				}
				int result = pclose(pipe);
				
				// Check if open command succeeded
				if (result == 0 && result_str.find("kLSApplicationNotFoundErr") == std::string::npos)
				{
					opened_successfully = true;
					Log::Info("Opened file: {}", full_path.string());
				}
			}
			
			// If simple open failed, use Finder to show native "Choose Application" dialog
			if (!opened_successfully)
			{
				std::string finder_command = "osascript -e 'tell application \"Finder\"' -e 'activate' -e 'open POSIX file \"" + full_path.string() + "\"' -e 'end tell'";
				int finder_result = system(finder_command.c_str());
				if (finder_result == 0)
				{
					Log::Info("Opened file with Finder dialog: {}", full_path.string());
				}
				else
				{
					Log::Error("Failed to open file: {}", full_path.string());
				}
			}
		#elif _WIN32
			// Windows: Use 'start' command
			std::string command = "start \"\" \"" + full_path.string() + "\"";
			int result = system(command.c_str());
			if (result == 0)
			{
				Log::Info("Opened file: {}", full_path.string());
			}
			else
			{
				Log::Error("Failed to open file: {}", full_path.string());
			}
		#else
			// Linux: Use 'xdg-open' command
			std::string command = "xdg-open \"" + full_path.string() + "\"";
			int result = system(command.c_str());
			if (result == 0)
			{
				Log::Info("Opened file: {}", full_path.string());
			}
			else
			{
				Log::Error("Failed to open file: {}", full_path.string());
			}
		#endif
	}

	void AssetBrowserAsset::Delete()
	{
		std::filesystem::path path_to_delete = Project::GetActiveAssetDirectory() / m_asset_info.FilePath;
		
		if (!std::filesystem::exists(path_to_delete))
		{
			Log::Error("Asset file does not exist: {}", path_to_delete.string());
			return;
		}
		
		try
		{
			bool deleted_successfully = false;
			
			// Move to Trash instead of permanent deletion
			#ifdef __APPLE__
				// macOS: Use osascript to move to Trash
				std::string command = "osascript -e 'tell application \"Finder\" to delete POSIX file \"" + path_to_delete.string() + "\"'";
				int result = system(command.c_str());
				if (result == 0)
				{
					Log::Info("Moved asset to Trash: {}", path_to_delete.string());
					deleted_successfully = true;
				}
				else
				{
					Log::Error("Failed to move asset to Trash. Please delete via Finder.");
				}
			#elif _WIN32
				// Windows: Use Shell API for fast Recycle Bin operation
				std::string path_str = path_to_delete.string();
				// SHFileOperation requires double-null terminated string
				std::vector<char> double_null_path(path_str.begin(), path_str.end());
				double_null_path.push_back('\0');
				double_null_path.push_back('\0');
				
				SHFILEOPSTRUCTA file_op = {};
				file_op.wFunc = FO_DELETE;
				file_op.pFrom = double_null_path.data();
				file_op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;
				
				int result = SHFileOperationA(&file_op);
				if (result == 0 && !file_op.fAnyOperationsAborted)
				{
					Log::Info("Moved asset to Recycle Bin: {}", path_to_delete.string());
					deleted_successfully = true;
				}
				else
				{
					Log::Error("Failed to move asset to Recycle Bin. Error code: {}", result);
				}
			#else
				// Linux: Try multiple trash methods
				std::string command = "gio trash \"" + path_to_delete.string() + "\" 2>/dev/null";
				int result = system(command.c_str());
				if (result == 0)
				{
					Log::Info("Moved asset to trash: {}", path_to_delete.string());
					deleted_successfully = true;
				}
				else
				{
					// Try trash-cli as fallback
					command = "trash-put \"" + path_to_delete.string() + "\" 2>/dev/null";
					result = system(command.c_str());
					if (result == 0)
					{
						Log::Info("Moved asset to trash: {}", path_to_delete.string());
						deleted_successfully = true;
					}
					else
					{
						Log::Error("Cannot delete asset: trash functionality not available. Please delete via file manager or install 'gio' or 'trash-cli'.");
					}
				}
			#endif
			
			// Only update registry and refresh if deletion was successful
			if (deleted_successfully)
			{
				// Remove from asset registry
				AssetManager::RemoveAsset(m_asset_info.Handle);
				AssetManager::SaveAssetRegistry(Project::GetActiveAssetRegistry());
				
				// Refresh parent directory to update UI
				if (m_panel)
				{
					m_panel->Refresh();
				}
			}
		}
		catch (const std::exception& e)
		{
			Log::Error("Failed to delete asset: {}", e.what());
		}
	}

	void AssetBrowserAsset::OnRenamed(const std::string& new_name)
	{
		if (new_name == m_file_name)
			return;
		
		std::filesystem::path old_path = Project::GetActiveAssetDirectory() / m_asset_info.FilePath;
		std::filesystem::path new_path = old_path.parent_path() / new_name;
		
		try
		{
			std::filesystem::rename(old_path, new_path);
			
			// Update asset metadata
			m_asset_info.FilePath = std::filesystem::relative(new_path, Project::GetActiveAssetDirectory());
			m_file_name = new_name;
			
			Log::Info("Renamed asset: {} -> {}", old_path.string(), new_path.string());
			
			// Update asset registry
			AssetManager::SaveAssetRegistry(Project::GetActiveAssetRegistry());
		}
		catch (const std::exception& e)
		{
			Log::Error("Failed to rename asset: {}", e.what());
		}
	}

} // namespace ignis
