#include "AssetBrowserItem.h"
#include "Editor/Panels/AssetBrowserPanel.h"
#include <imgui.h>

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
			if (m_is_selected)
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow for selected
			}

			ImGui::TextWrapped("%s", m_display_name.c_str());

			if (m_is_selected)
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
		// Handle single click selection (only if not double clicking)
		else if (ImGui::IsItemClicked())
		{
			m_is_selected = !m_is_selected;
		}

		// Handle right-click context menu
		if (ImGui::BeginPopupContextItem("item_context"))
		{
			if (ImGui::MenuItem("Rename"))
			{
				StartRenaming();
			}
			if (ImGui::MenuItem("Delete"))
			{
				Delete();
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
		// TODO: Implement directory deletion
		Log::Warn("Directory deletion not yet implemented: {}", m_file_name);
	}

	bool AssetBrowserDirectory::Move(const std::filesystem::path& destination)
	{
		// TODO: Implement directory moving
		Log::Warn("Directory moving not yet implemented: {}", m_file_name);
		return false;
	}

	void AssetBrowserDirectory::OnRenamed(const std::string& new_name)
	{
		// TODO: Implement directory renaming
		Log::Warn("Directory renaming not yet implemented: {} -> {}", m_file_name, new_name);
		m_file_name = new_name;
	}

	// AssetBrowserAsset implementation
	AssetBrowserAsset::AssetBrowserAsset(const AssetMetadata& asset_info)
		: AssetBrowserItem(ItemType::Asset, asset_info.Handle, asset_info.FilePath.filename().string())
		, m_asset_info(asset_info)
	{
	}

	void AssetBrowserAsset::Delete()
	{
		// TODO: Implement asset deletion
		Log::Warn("Asset deletion not yet implemented: {}", m_file_name);
	}

	bool AssetBrowserAsset::Move(const std::filesystem::path& destination)
	{
		// TODO: Implement asset moving
		Log::Warn("Asset moving not yet implemented: {}", m_file_name);
		return false;
	}

	void AssetBrowserAsset::OnRenamed(const std::string& new_name)
	{
		// TODO: Implement asset renaming
		Log::Warn("Asset renaming not yet implemented: {} -> {}", m_file_name, new_name);
		m_file_name = new_name;
	}

} // namespace ignis
