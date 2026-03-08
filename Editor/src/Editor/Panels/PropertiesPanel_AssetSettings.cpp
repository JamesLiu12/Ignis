#include "Editor/Panels/PropertiesPanel.h"
#include "Ignis/Asset/AssetManager.h"
#include "Ignis/Renderer/TextureTypes.h"
#include <imgui.h>

namespace ignis {

	// Helper to convert enum to string for dropdowns
	static const char* TextureFormatToString(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::R8: return "R8";
		case TextureFormat::RGBA8: return "RGBA8";
		case TextureFormat::RGBA8_sRGB: return "RGBA8 sRGB";
		case TextureFormat::RGBA16F: return "RGBA16F";
		case TextureFormat::RGBA32F: return "RGBA32F";
		case TextureFormat::RG16F: return "RG16F";
		case TextureFormat::RG32F: return "RG32F";
		case TextureFormat::Depth24: return "Depth24";
		case TextureFormat::Depth32F: return "Depth32F";
		case TextureFormat::Depth24Stencil8: return "Depth24Stencil8";
		default: return "Unknown";
		}
	}

	static const char* TextureWrapToString(TextureWrap wrap)
	{
		switch (wrap)
		{
		case TextureWrap::Repeat: return "Repeat";
		case TextureWrap::ClampToEdge: return "Clamp to Edge";
		case TextureWrap::ClampToBorder: return "Clamp to Border";
		case TextureWrap::MirroredRepeat: return "Mirrored Repeat";
		default: return "Unknown";
		}
	}

	static const char* TextureFilterToString(TextureFilter filter)
	{
		switch (filter)
		{
		case TextureFilter::Nearest: return "Nearest";
		case TextureFilter::Linear: return "Linear";
		case TextureFilter::NearestMipmapNearest: return "Nearest Mipmap Nearest";
		case TextureFilter::LinearMipmapNearest: return "Linear Mipmap Nearest";
		case TextureFilter::NearestMipmapLinear: return "Nearest Mipmap Linear";
		case TextureFilter::LinearMipmapLinear: return "Linear Mipmap Linear";
		default: return "Unknown";
		}
	}

	static const char* AssetTypeToString(AssetType type)
	{
		switch (type)
		{
		case AssetType::Texture2D: return "Texture 2D";
		case AssetType::TextureCube: return "Texture Cube";
		case AssetType::Mesh: return "Mesh";
		case AssetType::Font: return "Font";
		case AssetType::AudioClip: return "Audio Clip";
		case AssetType::EquirectIBLEnv: return "Equirect IBL Environment";
		default: return "Unknown";
		}
	}

	void PropertiesPanel::SetSelectedEntity(std::shared_ptr<Entity> entity)
	{
		m_selected_entity = entity;
		m_selected_asset = AssetHandle::Invalid;
		m_selected_unregistered_file.clear();
		m_asset_settings_modified = false;
	}

	void PropertiesPanel::SetSelectedAsset(AssetHandle handle)
	{
		m_selected_asset = handle;
		m_selected_entity.reset();
		m_selected_unregistered_file.clear();
		m_asset_settings_modified = false;

		if (handle.IsValid())
		{
			if (const AssetMetadata* meta = AssetManager::GetMetadata(handle))
				m_original_import_options = meta->ImportOptions;
		}
	}

	void PropertiesPanel::SetSelectedUnregisteredFile(const std::filesystem::path& path)
	{
		m_selected_unregistered_file = path;
		m_selected_asset = AssetHandle::Invalid;
		m_selected_entity.reset();
		m_asset_settings_modified = false;

		const AssetType inferred_type = InferAssetTypeFromPath(path);
		switch (inferred_type)
		{
		case AssetType::Texture2D:
			m_pending_import_options = TextureImportOptions{};
			break;
		case AssetType::EquirectIBLEnv: 
			m_pending_import_options = EquirectImportOptions{};
			break;
		case AssetType::Font:
			m_pending_import_options = FontImportOptions{};
			break;
		case AssetType::AudioClip:
			m_pending_import_options = AudioImportOptions{};
			break;
		default:
			m_pending_import_options = std::monostate{};
			break;
		}
	}

	void PropertiesPanel::RenderAssetProperties(AssetHandle handle)
	{
		const AssetMetadata* metadata = AssetManager::GetMetadata(handle);
		if (!metadata)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Invalid Asset Handle");
			return;
		}

		// Asset info header with color-coded type
		ImGui::Text("Asset Type:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", AssetTypeToString(metadata->Type));
		ImGui::Separator();
		
		ImGui::Text("File Path:");
		ImGui::TextWrapped("%s", metadata->FilePath.c_str());
		ImGui::Spacing();
		
		ImGui::Text("Handle: %llu", (uint64_t)handle);
		ImGui::Separator();
		ImGui::Spacing();

		// Import settings section
		ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Import Settings");
		if (m_asset_settings_modified)
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "*");
		}
		ImGui::Separator();

		// Get mutable metadata for editing
		AssetMetadata* mutable_metadata = AssetManager::GetMetadataMutable(handle);
		if (!mutable_metadata)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Cannot edit asset metadata");
			return;
		}

		// Render appropriate settings based on asset type
		std::visit(overloaded{
			[this](std::monostate&) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
				ImGui::TextWrapped("No import settings available for this asset type.");
				ImGui::PopStyleColor();
			},
			[this, handle](TextureImportOptions& opts) {
				RenderTextureImportSettings(opts, handle);
			},
			[this, handle](FontImportOptions& opts) {
				RenderFontImportSettings(opts, handle);
			},
			[this, handle](AudioImportOptions& opts) {
				RenderAudioImportSettings(opts, handle);
			},
			[this, handle](EquirectImportOptions& opts) {
				RenderEquirectImportSettings(opts, handle);
			},
		}, mutable_metadata->ImportOptions);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// Apply Changes button
		ImGui::BeginDisabled(!m_asset_settings_modified);
		if (ImGui::Button("Apply Changes", ImVec2(-1, 0)))
		{
			ReimportAsset(handle);
			m_asset_settings_modified = false;
			
			// Update original settings to current (after successful apply)
			if (const AssetMetadata* updated_meta = AssetManager::GetMetadata(handle))
			{
				m_original_import_options = updated_meta->ImportOptions;
			}
			
			Log::CoreInfo("Applied import settings for asset: {}", metadata->FilePath);
		}
		ImGui::EndDisabled();

		if (m_asset_settings_modified)
		{
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Settings modified - click Apply to save");
		}
		else
		{
			ImGui::TextDisabled("No changes to apply");
		}
	}

	void PropertiesPanel::RenderTextureImportSettings(TextureImportOptions& opts, AssetHandle handle)
	{
		bool modified = false;

		// Flip Vertical
		if (ImGui::Checkbox("Flip Vertical", &opts.FlipVertical))
			modified = true;

		// Generate Mipmaps
		if (ImGui::Checkbox("Generate Mipmaps", &opts.GenMipmaps))
			modified = true;

		ImGui::Spacing();

		// Internal Format dropdown
		const char* formats[] = { "R8", "RGBA8", "RGBA8 sRGB", "RGBA16F", "RGBA32F", "RG16F", "RG32F", "Depth24", "Depth32F", "Depth24Stencil8" };
		const TextureFormat format_values[] = { 
			TextureFormat::R8, TextureFormat::RGBA8, TextureFormat::RGBA8_sRGB, 
			TextureFormat::RGBA16F, TextureFormat::RGBA32F, TextureFormat::RG16F, 
			TextureFormat::RG32F, TextureFormat::Depth24, TextureFormat::Depth32F, 
			TextureFormat::Depth24Stencil8 
		};
		
		// Find current index
		int current_format = 0;
		for (int i = 0; i < IM_ARRAYSIZE(format_values); i++)
		{
			if (format_values[i] == opts.InternalFormat)
			{
				current_format = i;
				break;
			}
		}
		
		if (ImGui::Combo("Internal Format", &current_format, formats, IM_ARRAYSIZE(formats)))
		{
			opts.InternalFormat = format_values[current_format];
			modified = true;
		}

		ImGui::Spacing();

		// Wrap S dropdown
		const char* wraps[] = { "Repeat", "Clamp to Edge", "Clamp to Border", "Mirrored Repeat" };
		int current_wrap_s = static_cast<int>(opts.WrapS);
		if (ImGui::Combo("Wrap S", &current_wrap_s, wraps, IM_ARRAYSIZE(wraps)))
		{
			opts.WrapS = static_cast<TextureWrap>(current_wrap_s);
			modified = true;
		}

		// Wrap T dropdown
		int current_wrap_t = static_cast<int>(opts.WrapT);
		if (ImGui::Combo("Wrap T", &current_wrap_t, wraps, IM_ARRAYSIZE(wraps)))
		{
			opts.WrapT = static_cast<TextureWrap>(current_wrap_t);
			modified = true;
		}

		ImGui::Spacing();

		// Min Filter dropdown
		const char* filters[] = { "Nearest", "Linear", "Nearest Mipmap Nearest", 
		                          "Linear Mipmap Nearest", "Nearest Mipmap Linear", "Linear Mipmap Linear" };
		int current_min_filter = static_cast<int>(opts.MinFilter);
		if (ImGui::Combo("Min Filter", &current_min_filter, filters, IM_ARRAYSIZE(filters)))
		{
			opts.MinFilter = static_cast<TextureFilter>(current_min_filter);
			modified = true;
		}

		// Mag Filter dropdown (only Nearest and Linear)
		const char* mag_filters[] = { "Nearest", "Linear" };
		int current_mag_filter = static_cast<int>(opts.MagFilter);
		if (ImGui::Combo("Mag Filter", &current_mag_filter, mag_filters, IM_ARRAYSIZE(mag_filters)))
		{
			opts.MagFilter = static_cast<TextureFilter>(current_mag_filter);
			modified = true;
		}

		if (modified)
		{
			// Check if current settings match original
			if (std::holds_alternative<TextureImportOptions>(m_original_import_options))
			{
				auto& original = std::get<TextureImportOptions>(m_original_import_options);
				m_asset_settings_modified = !(opts.FlipVertical == original.FlipVertical &&
					opts.GenMipmaps == original.GenMipmaps &&
					opts.InternalFormat == original.InternalFormat &&
					opts.WrapS == original.WrapS &&
					opts.WrapT == original.WrapT &&
					opts.MinFilter == original.MinFilter &&
					opts.MagFilter == original.MagFilter);
			}
			else
			{
				m_asset_settings_modified = true;
			}
		}
	}

	void PropertiesPanel::RenderFontImportSettings(FontImportOptions& opts, AssetHandle handle)
	{
		bool modified = false;

		// Font Size slider (8.0 - 256.0) with clamping
		float font_size = opts.FontSize;
		if (ImGui::SliderFloat("Font Size", &font_size, 8.0f, 256.0f, "%.1f"))
		{
			// Clamp to valid range
			font_size = std::clamp(font_size, 8.0f, 256.0f);
			opts.FontSize = font_size;
			modified = true;
		}

		ImGui::Spacing();

		// Atlas Width slider (256 - 4096) with validation
		int atlas_width = static_cast<int>(opts.AtlasWidth);
		if (ImGui::SliderInt("Atlas Width", &atlas_width, 256, 4096))
		{
			// Clamp to valid range
			atlas_width = std::clamp(atlas_width, 256, 4096);
			opts.AtlasWidth = static_cast<uint32_t>(atlas_width);
			modified = true;
		}
		
		// Show warning if not power of 2
		if ((atlas_width & (atlas_width - 1)) != 0)
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "!");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Warning: Not a power of 2 (may reduce performance)");
		}

		// Atlas Height slider (256 - 4096) with validation
		int atlas_height = static_cast<int>(opts.AtlasHeight);
		if (ImGui::SliderInt("Atlas Height", &atlas_height, 256, 4096))
		{
			// Clamp to valid range
			atlas_height = std::clamp(atlas_height, 256, 4096);
			opts.AtlasHeight = static_cast<uint32_t>(atlas_height);
			modified = true;
		}
		
		// Show warning if not power of 2
		if ((atlas_height & (atlas_height - 1)) != 0)
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "!");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Warning: Not a power of 2 (may reduce performance)");
		}

		if (modified)
		{
			// Check if current settings match original
			if (std::holds_alternative<FontImportOptions>(m_original_import_options))
			{
				auto& original = std::get<FontImportOptions>(m_original_import_options);
				m_asset_settings_modified = !(opts.FontSize == original.FontSize &&
					opts.AtlasWidth == original.AtlasWidth &&
					opts.AtlasHeight == original.AtlasHeight);
			}
			else
			{
				m_asset_settings_modified = true;
			}
		}
	}

	void PropertiesPanel::RenderAudioImportSettings(AudioImportOptions& opts, AssetHandle handle)
	{
		bool modified = false;

		// Stream checkbox
		if (ImGui::Checkbox("Stream Audio", &opts.Stream))
			modified = true;

		ImGui::TextDisabled("Enable streaming for large audio files to reduce memory usage");

		if (modified)
		{
			// Check if current settings match original
			if (std::holds_alternative<AudioImportOptions>(m_original_import_options))
			{
				auto& original = std::get<AudioImportOptions>(m_original_import_options);
				m_asset_settings_modified = !(opts.Stream == original.Stream);
			}
			else
			{
				m_asset_settings_modified = true;
			}
		}
	}

	void PropertiesPanel::RenderEquirectImportSettings(EquirectImportOptions& opts, AssetHandle handle)
	{
		bool modified = false;

		ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Texture Options");
		ImGui::Separator();

		// Reuse texture import settings rendering
		if (ImGui::Checkbox("Flip Vertical##equirect", &opts.TexOptions.FlipVertical))
			modified = true;

		if (ImGui::Checkbox("Generate Mipmaps##equirect", &opts.TexOptions.GenMipmaps))
			modified = true;

		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "IBL Bake Settings");
		ImGui::Separator();

		// IBL Resolution sliders with validation
		int environment_res = static_cast<int>(opts.BakeSettings.EnvironmentResolution);
		if (ImGui::SliderInt("Environment Resolution", &environment_res, 128, 2048))
		{
			environment_res = std::clamp(environment_res, 128, 2048);
			opts.BakeSettings.EnvironmentResolution = static_cast<uint32_t>(environment_res);
			modified = true;
		}
		if ((environment_res & (environment_res - 1)) != 0)
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "!");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Recommended: Use power of 2 for optimal performance");
		}

		int irradiance_res = static_cast<int>(opts.BakeSettings.IrradianceResolution);
		if (ImGui::SliderInt("Irradiance Resolution", &irradiance_res, 16, 512))
		{
			irradiance_res = std::clamp(irradiance_res, 16, 512);
			opts.BakeSettings.IrradianceResolution = static_cast<uint32_t>(irradiance_res);
			modified = true;
		}
		if ((irradiance_res & (irradiance_res - 1)) != 0)
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "!");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Recommended: Use power of 2 for optimal performance");
		}

		int prefilter_res = static_cast<int>(opts.BakeSettings.PrefilterResolution);
		if (ImGui::SliderInt("Prefilter Resolution", &prefilter_res, 16, 1024))
		{
			prefilter_res = std::clamp(prefilter_res, 16, 1024);
			opts.BakeSettings.PrefilterResolution = static_cast<uint32_t>(prefilter_res);
			modified = true;
		}
		if ((prefilter_res & (prefilter_res - 1)) != 0)
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "!");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Recommended: Use power of 2 for optimal performance");
		}

		int brdf_res = static_cast<int>(opts.BakeSettings.BrdfLUTResolution);
		if (ImGui::SliderInt("BRDF LUT Resolution", &brdf_res, 128, 1024))
		{
			brdf_res = std::clamp(brdf_res, 128, 1024);
			opts.BakeSettings.BrdfLUTResolution = static_cast<uint32_t>(brdf_res);
			modified = true;
		}
		if ((brdf_res & (brdf_res - 1)) != 0)
		{
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "!");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Recommended: Use power of 2 for optimal performance");
		}

		if (modified)
		{
			// Check if current settings match original
			if (std::holds_alternative<EquirectImportOptions>(m_original_import_options))
			{
				auto& original = std::get<EquirectImportOptions>(m_original_import_options);
				m_asset_settings_modified = !(opts.TexOptions.FlipVertical == original.TexOptions.FlipVertical &&
					opts.TexOptions.GenMipmaps == original.TexOptions.GenMipmaps &&
					opts.BakeSettings.EnvironmentResolution == original.BakeSettings.EnvironmentResolution &&
					opts.BakeSettings.IrradianceResolution == original.BakeSettings.IrradianceResolution &&
					opts.BakeSettings.PrefilterResolution == original.BakeSettings.PrefilterResolution &&
					opts.BakeSettings.BrdfLUTResolution == original.BakeSettings.BrdfLUTResolution);
			}
			else
			{
				m_asset_settings_modified = true;
			}
		}
	}

} // namespace ignis
