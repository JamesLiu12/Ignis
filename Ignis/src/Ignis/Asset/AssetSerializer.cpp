#include "AssetSerializer.h"
#include "Ignis/Core/File/File.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

namespace ignis
{
	static ordered_json SerializeTextureImportOptions(const TextureImportOptions& opts)
	{
		ordered_json data;
		data["FlipVertical"] = opts.FlipVertical;
		data["GenMipmaps"] = opts.GenMipmaps;
		data["InternalFormat"] = static_cast<int>(opts.InternalFormat);
		data["WrapS"] = static_cast<int>(opts.WrapS);
		data["WrapT"] = static_cast<int>(opts.WrapT);
		data["MinFilter"] = static_cast<int>(opts.MinFilter);
		data["MagFilter"] = static_cast<int>(opts.MagFilter);
		return data;
	}

	static TextureImportOptions DeserializeTextureImportOptions(const ordered_json& data)
	{
		TextureImportOptions opts{};
		opts.FlipVertical = data.value("FlipVertical", opts.FlipVertical);
		opts.GenMipmaps = data.value("GenMipmaps", opts.GenMipmaps);
		opts.InternalFormat = static_cast<TextureFormat>(data.value("InternalFormat", static_cast<int>(opts.InternalFormat)));
		opts.WrapS = static_cast<TextureWrap>  (data.value("WrapS", static_cast<int>(opts.WrapS)));
		opts.WrapT = static_cast<TextureWrap>  (data.value("WrapT", static_cast<int>(opts.WrapT)));
		opts.MinFilter = static_cast<TextureFilter>(data.value("MinFilter", static_cast<int>(opts.MinFilter)));
		opts.MagFilter = static_cast<TextureFilter>(data.value("MagFilter", static_cast<int>(opts.MagFilter)));
		return opts;
	}

	static ordered_json SerializeFontImportOptions(const FontImportOptions& opts)
	{
		ordered_json data;
		data["FontSize"] = opts.FontSize;
		data["AtlasWidth"] = opts.AtlasWidth;
		data["AtlasHeight"] = opts.AtlasHeight;
		return data;
	}

	static FontImportOptions DeserializeFontImportOptions(const ordered_json& data)
	{
		FontImportOptions opts{};
		opts.FontSize = data.value("FontSize", opts.FontSize);
		opts.AtlasWidth = data.value("AtlasWidth", opts.AtlasWidth);
		opts.AtlasHeight = data.value("AtlasHeight", opts.AtlasHeight);
		return opts;
	}

	static ordered_json SerializeAudioImportOptions(const AudioImportOptions& opts)
	{
		ordered_json data;
		data["Stream"] = opts.Stream;
		return data;
	}

	static AudioImportOptions DeserializeAudioImportOptions(const ordered_json& data)
	{
		AudioImportOptions opts{};
		opts.Stream = data.value("Stream", opts.Stream);
		return opts;
	}

	static ordered_json SerializeIBLBakeSettings(const IBLBakeSettings& settings)
	{
		ordered_json data;
		data["EnvironmentResolution"] = settings.EnvironmentResolution;
		data["IrradianceResolution"] = settings.IrradianceResolution;
		data["PrefilterResolution"] = settings.PrefilterResolution;
		data["BrdfLUTResolution"] = settings.BrdfLUTResolution;
		return data;
	}

	static IBLBakeSettings DeserializeIBLBakeSettings(const ordered_json& data)
	{
		IBLBakeSettings settings{};
		settings.EnvironmentResolution = data.value("EnvironmentResolution", settings.EnvironmentResolution);
		settings.IrradianceResolution = data.value("IrradianceResolution", settings.IrradianceResolution);
		settings.PrefilterResolution = data.value("PrefilterResolution", settings.PrefilterResolution);
		settings.BrdfLUTResolution = data.value("BrdfLUTResolution", settings.BrdfLUTResolution);
		return settings;
	}

	static ordered_json SerializeEquirectImportOptions(const EquirectImportOptions& opts)
	{
		ordered_json data;
		data["TexOptions"] = SerializeTextureImportOptions(opts.TexOptions);
		data["BakeSettings"] = SerializeIBLBakeSettings(opts.BakeSettings);
		return data;
	}

	static EquirectImportOptions DeserializeEquirectImportOptions(const ordered_json& data)
	{
		EquirectImportOptions opts{};
		if (data.contains("TexOptions"))
			opts.TexOptions = DeserializeTextureImportOptions(data["TexOptions"]);
		if (data.contains("BakeSettings"))
			opts.BakeSettings = DeserializeIBLBakeSettings(data["BakeSettings"]);
		return opts;
	}

	static ordered_json SerializeImportOptions(const AssetImportOptions& options)
	{
		ordered_json data;

		if (std::holds_alternative<TextureImportOptions>(options))
		{
			data["OptionsType"] = "Texture";
			data["Options"] = SerializeTextureImportOptions(std::get<TextureImportOptions>(options));
		}
		else if (std::holds_alternative<FontImportOptions>(options))
		{
			data["OptionsType"] = "Font";
			data["Options"] = SerializeFontImportOptions(std::get<FontImportOptions>(options));
		}
		else if (std::holds_alternative<AudioImportOptions>(options))
		{
			data["OptionsType"] = "Audio";
			data["Options"] = SerializeAudioImportOptions(std::get<AudioImportOptions>(options));
		}
		else if (std::holds_alternative<EquirectImportOptions>(options))
		{
			data["OptionsType"] = "Equirect";
			data["Options"] = SerializeEquirectImportOptions(std::get<EquirectImportOptions>(options));
		}
		else
		{
			data["OptionsType"] = "None";
		}

		return data;
	}

	static AssetImportOptions DeserializeImportOptions(const ordered_json& data)
	{
		if (!data.contains("OptionsType"))
			return std::monostate{};

		std::string type = data.value("OptionsType", "None");

		if (type == "Texture" && data.contains("Options"))
			return DeserializeTextureImportOptions(data["Options"]);
		if (type == "Font" && data.contains("Options"))
			return DeserializeFontImportOptions(data["Options"]);
		if (type == "Audio" && data.contains("Options"))
			return DeserializeAudioImportOptions(data["Options"]);
		if (type == "Equirect" && data.contains("Options"))
			return DeserializeEquirectImportOptions(data["Options"]);

		return std::monostate{};
	}

	static ordered_json SerializeMetadata(const AssetMetadata& meta)
	{
		ordered_json data;
		data["Handle"] = meta.Handle.ToString();
		data["Type"] = static_cast<int>(meta.Type);
		data["FilePath"] = FileSystem::ToUnixPath(meta.FilePath);
		data["ImportOptions"] = SerializeImportOptions(meta.ImportOptions);
		return data;
	}

	static bool DeserializeMetadata(const ordered_json& data, AssetMetadata& out_meta)
	{
		if (!data.contains("Handle") || !data["Handle"].is_string())   return false;
		if (!data.contains("Type") || !data["Type"].is_number_integer()) return false;
		if (!data.contains("FilePath") || !data["FilePath"].is_string()) return false;

		out_meta.Handle = UUID(data["Handle"].get<std::string>());
		out_meta.Type = static_cast<AssetType>(data["Type"].get<int>());
		out_meta.FilePath = data["FilePath"].get<std::string>();

		if (data.contains("ImportOptions"))
			out_meta.ImportOptions = DeserializeImportOptions(data["ImportOptions"]);

		return true;
	}

	bool AssetSerializer::Serialize(const std::unordered_map<AssetHandle, AssetMetadata>& asset_registry, const std::filesystem::path& filepath)
	{
		File file(filepath);
		auto stream = file.OpenOutputStream();
		if (!stream.is_open())
		{
			return false;
			Log::CoreError("[AssetSerializer::Serialize] Failed to open file for writing");
		}

		ordered_json data;
		data["Assets"] = ordered_json::array();

		std::vector<std::pair<AssetHandle, AssetMetadata>> sorted_assets(
			asset_registry.begin(),
			asset_registry.end()
		);

		std::sort(sorted_assets.begin(), sorted_assets.end(),
			[](const auto& a, const auto& b) {
				return a.first < b.first;
			});

		for (const auto& [asset_handle, meta_data] : sorted_assets)
		{
			AssetMetadata meta = meta_data;
			meta.Handle = asset_handle;

			data["Assets"].push_back(SerializeMetadata(meta));
		}

		try
		{
			stream << data.dump(4);
			stream.close();
			Log::CoreInfo("[AssetSerializer::Serialize] Successfully serialized asset to: {}", file.GetPath().string());
			return true;
		}
		catch (const std::exception& e)
		{
			Log::CoreError("[AssetSerializer::Serialize] Failed to write JSON: {}", e.what());
			return false;
		}
	}

	std::optional<std::unordered_map<AssetHandle, AssetMetadata>> AssetSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		std::unordered_map<AssetHandle, AssetMetadata> registry;

		File file(filepath);
		auto stream = file.OpenInputStream();
		if (!stream.is_open())
		{
			Log::CoreError("[AssetSerializer::Deserialize] Failed to open file for reading: {}",
				filepath.string());
			return {};
		}

		ordered_json data;
		try
		{
			stream >> data;
		}
		catch (const std::exception& e)
		{
			Log::CoreError("[AssetSerializer::Deserialize] Failed to parse JSON: {}", e.what());
			return {};
		}

		if (!data.contains("Assets") || !data["Assets"].is_array())
		{
			Log::CoreError("[AssetSerializer::Deserialize] Invalid format: missing 'Assets' array");
			return {};
		}

		for (const auto& item : data["Assets"])
		{
			AssetMetadata meta;
			if (!DeserializeMetadata(item, meta))
			{
				Log::CoreError("[AssetSerializer::Deserialize] Skipping invalid asset entry");
				continue;
			}

			const AssetHandle handle = meta.Handle;
			registry[handle] = std::move(meta);
		}

		Log::CoreInfo("[AssetSerializer::Deserialize] Successfully deserialized assets from: {}",
			file.GetPath().string());
		return registry;
	}
}